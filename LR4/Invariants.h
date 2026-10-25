#ifndef INVARIANTS_H
#define INVARIANTS_H

#include <windows.h>
#include <vector>
#include <atomic>

using namespace std;

class InvariantChecker {
public:
    InvariantChecker(int numPhilosophers, int numForks);
    ~InvariantChecker();

    void checkForkExclusivity(int forkId, int philosopherId);

    void checkAllForksReleased();

    void checkEatingRequiresBothForks(int philosopherId, bool hasLeft, bool hasRight, bool isEating);

    int getViolationCount() const { return violationCount_; }

private:
    CRITICAL_SECTION cs_;
    int numPhilosophers_;
    int numForks_;
    atomic<int> violationCount_;
    vector<bool> forkHeld_;
    vector<int> forkHolder_;
};

#endif