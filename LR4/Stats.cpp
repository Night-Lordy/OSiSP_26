#include "Stats.h"
#include <iostream>
#include <iomanip>

using namespace std;

Stats::Stats(int numPhilosophers) {
    InitializeCriticalSection(&cs_);
    stats_.resize(numPhilosophers);
    for (int i = 0; i < numPhilosophers; i++) {
        stats_[i].id = i;
        stats_[i].cyclesCompleted = 0;
        stats_[i].totalThinkingTime = 0;
        stats_[i].totalEatingTime = 0;
        stats_[i].totalWaitingTime = 0;
        stats_[i].timeoutCount = 0;
        stats_[i].invariantViolations = 0;
    }
}

Stats::~Stats() {
    DeleteCriticalSection(&cs_);
}

void Stats::recordCycle(int id, int thinkingMs, int eatingMs, int waitingMs, bool timeout) {
    EnterCriticalSection(&cs_);
    if (id >= 0 && id < (int)stats_.size()) {
        stats_[id].cyclesCompleted++;
        stats_[id].totalThinkingTime += thinkingMs;
        stats_[id].totalEatingTime += eatingMs;
        stats_[id].totalWaitingTime += waitingMs;
        if (timeout) stats_[id].timeoutCount++;
    }
    LeaveCriticalSection(&cs_);
}

void Stats::recordInvariantViolation(int id) {
    EnterCriticalSection(&cs_);
    if (id >= 0 && id < (int)stats_.size()) {
        stats_[id].invariantViolations++;
    }
    LeaveCriticalSection(&cs_);
}

void Stats::printSummary() const {
    cout << "\n========== Statistic ==========\n";
    cout << left << setw(6) << "ID"
        << setw(10) << "Cycles"
        << setw(12) << "Thinking"
        << setw(10) << "Food"
        << setw(12) << "Waiting"
        << setw(10) << "Time out"
        << setw(12) << "Violations" << "\n";
    cout << string(62, '-') << "\n";

    int totalViolations = 0;
    for (const auto& s : stats_) {
        cout << left << setw(6) << s.id
            << setw(10) << s.cyclesCompleted
            << setw(12) << (to_string(s.totalThinkingTime) + " ms")
            << setw(10) << (to_string(s.totalEatingTime) + " ms")
            << setw(12) << (to_string(s.totalWaitingTime) + " ms")
            << setw(10) << s.timeoutCount
            << setw(12) << s.invariantViolations << "\n";
        totalViolations += s.invariantViolations;
    }
    cout << "\nTotal invariant violations: " << totalViolations << "\n";
}

int Stats::getTotalInvariantViolations() const {
    int total = 0;
    for (const auto& s : stats_) total += s.invariantViolations;
    return total;
}