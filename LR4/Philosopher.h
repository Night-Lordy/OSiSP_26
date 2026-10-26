#ifndef PHILOSOPHER_H
#define PHILOSOPHER_H

#include <windows.h>
#include "Fork.h"
#include "Logger.h"
#include "Stats.h"
#include "Invariants.h"

enum class PhilosopherState {
    THINKING,
    WAITING,
    EATING,
    FINISHED
};

struct PhilosopherConfig {
    int id;
    int numCycles;
    int thinkingTimeMs;
    int eatingTimeMs;
    DWORD maxWaitTimeMs;
    int numForks;
};

class Philosopher {
    public:
        Philosopher(const PhilosopherConfig& config, Fork* forks,
                    Logger* logger, Stats* stats, InvariantChecker* invariants);
        ~Philosopher();

        void run();
        PhilosopherState getState() const { return state_; }

        static DWORD WINAPI threadProc(LPVOID param);

    private:
        PhilosopherConfig config_;
        Fork* forks_;
        Logger* logger_;
        Stats* stats_;
        InvariantChecker* invariants_;
        PhilosopherState state_;
        HANDLE threadHandle_;
        DWORD tid_;

        int getLeftFork() const;
        int getRightFork() const;
        void setState(PhilosopherState newState);
};

#endif