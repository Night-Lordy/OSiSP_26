#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "Philosopher.h"
#include "Fork.h"
#include "Logger.h"
#include "Stats.h"
#include "Invariants.h"

using namespace std;

struct Config {
    int numPhilosophers = 5;
    int numCycles = 100;
    int thinkingTimeMs = 2;
    int eatingTimeMs = 5;
    DWORD maxWaitTimeMs = 5000;
    DWORD experimentTimeoutMs = 30000;
    bool logEnabled = true;
    bool verbose = false;

    int studentId = 0;
    int seed = 1000;           
    int operations = 100;

    int numRuns = 3;
};

Config readConfig(const string& filename) {
    Config cfg;
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Warning: config file not found, using defaults.\n";
        return cfg;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        auto eqPos = line.find('=');
        if (eqPos == string::npos) continue;

        string key = line.substr(0, eqPos);
        string value = line.substr(eqPos + 1);

        while (!key.empty() && key.back() == ' ') key.pop_back();
        while (!value.empty() && value.front() == ' ') value.erase(value.begin());

        if (key == "num_philosophers") cfg.numPhilosophers = stoi(value);
        else if (key == "num_cycles") cfg.numCycles = stoi(value);
        else if (key == "thinking_time_ms") cfg.thinkingTimeMs = stoi(value);
        else if (key == "eating_time_ms") cfg.eatingTimeMs = stoi(value);
        else if (key == "max_wait_time_ms") cfg.maxWaitTimeMs = stoul(value);
        else if (key == "experiment_timeout_ms") cfg.experimentTimeoutMs = stoul(value);
        else if (key == "log_enabled") cfg.logEnabled = (value == "1");
        else if (key == "verbose") cfg.verbose = (value == "1");
    }

    cfg.seed = 1000 + cfg.studentId * 97;
    cfg.operations = cfg.numCycles + cfg.studentId * 5;

    return cfg;
}

struct ExperimentResult {
    string expName;
    int numPhilosophers;
    int thinkingTimeMs;
    int numCycles;
    int totalDurationMs;
    int invariantViolations;
    bool deadlockDetected;

    int totalSuccessfulOps;    
    int totalTimeouts;         
    int totalWaitingTimeMs;    
    double avgCycleTimeMs;

    int runNum;
};

ExperimentResult runExperiment(const Config& baseConfig, int numPhilosophers,
                               int thinkingTimeMs, const string& expName,
                               int runNumber, int numCycles) {

    cout << "\n----------------------------------------\n";
    cout << "Experiment: " << expName << " (run " << runNumber << ")\n";
    cout << "Philosophers: " << numPhilosophers << ", Cycles: " << numCycles << ", Thinking: " << thinkingTimeMs << " ms\n";
    cout << "----------------------------------------\n";

    Config cfg = baseConfig;
    cfg.numPhilosophers = numPhilosophers;
    cfg.thinkingTimeMs = thinkingTimeMs;
    cfg.numCycles = numCycles;

    vector<Fork> forks;
    for (int i = 0; i < cfg.numPhilosophers; i++) {
        forks.emplace_back(i);
    }

    Logger logger;
    string logFile = "experiment_" + expName + "_run" + to_string(runNumber) + ".log";
    logger.init(cfg.logEnabled, cfg.verbose, logFile);

    Stats stats(cfg.numPhilosophers);
    InvariantChecker invariants(cfg.numPhilosophers, cfg.numPhilosophers);

    vector<Philosopher*> philosophers;
    vector<HANDLE> threads;

    for (int i = 0; i < cfg.numPhilosophers; i++) {
        PhilosopherConfig pConfig;
        pConfig.id = i;
        pConfig.numCycles = cfg.numCycles;
        pConfig.thinkingTimeMs = cfg.thinkingTimeMs;
        pConfig.eatingTimeMs = cfg.eatingTimeMs;
        pConfig.maxWaitTimeMs = cfg.maxWaitTimeMs;
        pConfig.numForks = cfg.numPhilosophers;

        auto* p = new Philosopher(pConfig, forks.data(), &logger, &stats, &invariants);
        philosophers.push_back(p);

        HANDLE hThread = CreateThread(NULL, 0, Philosopher::threadProc, p, 0, NULL);
        if (hThread == NULL) {
            cout << "Error creating thread for philosopher " << i
                << ": " << GetLastError() << endl;
            delete p;
            continue;
        }
        threads.push_back(hThread);
    }

    DWORD startExp = GetTickCount();
    DWORD waitResult = WaitForMultipleObjects(
        (DWORD)threads.size(),
        threads.data(),
        TRUE,
        cfg.experimentTimeoutMs
    );

    DWORD totalDuration = GetTickCount() - startExp;

    bool deadlockDetected = false;
    if (waitResult == WAIT_TIMEOUT) {
        cout << "\n[ERROR] Experiment timed out! Possible deadlock.\n";
        deadlockDetected = true;

        for (auto h : threads) {
            TerminateThread(h, 1);
            CloseHandle(h);
        }
    }
    else if (waitResult == WAIT_FAILED) {
        cout << "WaitForMultipleObjects failed: " << GetLastError() << endl;
    }

    invariants.checkAllForksReleased();

    int totalSuccessfulOps = 0;
    int totalTimeouts = 0;
    int totalWaitingTime = 0;

    totalSuccessfulOps = numCycles * numPhilosophers;

    ExperimentResult result;
    result.expName = expName;
    result.numPhilosophers = numPhilosophers;
    result.thinkingTimeMs = thinkingTimeMs;
    result.numCycles = numCycles;
    result.totalDurationMs = totalDuration;
    result.invariantViolations = invariants.getViolationCount();
    result.deadlockDetected = deadlockDetected;
    result.totalSuccessfulOps = totalSuccessfulOps;
    result.totalTimeouts = totalTimeouts;
    result.totalWaitingTimeMs = totalWaitingTime;
    result.avgCycleTimeMs = (totalDuration > 0) ?
        (double)totalDuration / (numCycles * numPhilosophers) : 0;
    result.runNum = runNumber;

    for (auto p : philosophers) delete p;

    return result;
}

int main(int argc, char* argv[]) {
    string configFile = "config.txt";
    if (argc > 1) configFile = argv[1];

    Config baseConfig = readConfig(configFile);

    cout << "Configuration:\n";
    cout << "  STUDENT_ID: " << baseConfig.studentId << "\n";
    cout << "  SEED: " << baseConfig.seed << "\n";
    cout << "  BASE_OPERATIONS: " << baseConfig.numCycles << "\n";
    cout << "  OPERATIONS (indiv.): " << baseConfig.operations << "\n";
    cout << "  Philosophers (for indiv.): " << baseConfig.numPhilosophers << "\n";
    cout << "  Cycles (by def.): " << baseConfig.numCycles << "\n";
    cout << "  Thinking time (by def.): " << baseConfig.thinkingTimeMs << " ms\n";
    cout << "  Eating time: " << baseConfig.eatingTimeMs << " ms\n";
    cout << "  Wait time: " << baseConfig.maxWaitTimeMs << " ms\n";
    cout << "  Experiment time: " << baseConfig.experimentTimeoutMs << " ms\n";
    cout << "  Number of runs: " << baseConfig.numRuns << '\n';

    int runs = baseConfig.numRuns;
    vector<ExperimentResult> results;

    int philosopherCounts[] = { 5, 7 };
    int thinkingTimes[] = { 2, 15 };

    for (int np : philosopherCounts) {
        for (int tt : thinkingTimes) {
            for (int run = 1; run <= runs; ++run) {
                string expName = "P" + to_string(np) + "_T" + to_string(tt) + "ms";
                ExperimentResult res = runExperiment(baseConfig, np, tt, expName, run, baseConfig.numCycles);
                results.push_back(res);
            }
        }
    }
    
    for (int run = 1; run <= runs; ++run) {
        string expName = "Indiv";
        ExperimentResult res = runExperiment(baseConfig, baseConfig.numPhilosophers, baseConfig.thinkingTimeMs, expName, run, baseConfig.operations);
        results.push_back(res);
    }

    cout << "\n\n";
    cout << "=================================================================\n";
    cout << "|                        Final report                           |\n";
    cout << "|===============================================================|\n";
    cout << "| Experiment        | Phi.  | Think.  | Time     | Vio.  | Run  |\n";
    cout << "|===============================================================|\n";

    int totalViolations = 0;
    bool anyDeadlock = false;

    for (const auto& r : results) {
        char buf[256];
        snprintf(buf, sizeof(buf),
            "| %17s | %5d | %4d ms | %5d ms | %5d | %4d |",
            r.expName.c_str(), r.numPhilosophers, r.thinkingTimeMs,
            r.totalDurationMs, r.invariantViolations, r.runNum);
        cout << buf << "\n";
        totalViolations += r.invariantViolations;
        if (r.deadlockDetected) anyDeadlock = true;
    }

    cout << "================================================================\n";
    cout << "\nTotal invariant violations: " << totalViolations << "\n";
    cout << "Mutual blocking of detections: " << (anyDeadlock ? "Yes" : "No") << "\n";

    if (totalViolations == 0 && !anyDeadlock) {
        cout << "\n[OK] All experiments were completed correctly.\n";
        cout << "There is no mutual blocking.\n";
    }
    else {
        cout << "\n[FAIL] Violations or mutual blocking were detected.\n";
    }
     return (totalViolations == 0 && !anyDeadlock) ? 0 : 1;
}