#ifndef STATS_H
#define STATS_H

#include <windows.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

struct PhilosopherStats {
    int id;
    int cyclesCompleted;
    int totalThinkingTime;
    int totalEatingTime;
    int totalWaitingTime;
    int timeoutCount;
    int invariantViolations;
};

class Stats {
    public:
        Stats(int numPhilosophers);
        ~Stats();

        void recordCycle(int id, int thinkingMs, int eatingMs, int waitingMs, bool timeout);
        void recordInvariantViolation(int id);
        void printSummary() const;
        int getTotalInvariantViolations() const;

    private:
        vector<PhilosopherStats> stats_;
        CRITICAL_SECTION cs_;
};

#endif