#include "Invariants.h"
#include <iostream>

using namespace std;

InvariantChecker::InvariantChecker(int numPhilosophers, int numForks)
    : numPhilosophers_(numPhilosophers), numForks_(numForks), violationCount_(0) {
    InitializeCriticalSection(&cs_);
    forkHeld_.resize(numForks, false);
    forkHolder_.resize(numForks, -1);
}

InvariantChecker::~InvariantChecker() {
    DeleteCriticalSection(&cs_);
}

void InvariantChecker::checkForkExclusivity(int forkId, int philosopherId) {
    EnterCriticalSection(&cs_);
    if (forkId >= 0 && forkId < numForks_) {
        if (forkHeld_[forkId] && forkHolder_[forkId] != philosopherId) {
            violationCount_++;
            cout << "[INVARIANT VIOLATION] Fork " << forkId << " held by P" << forkHolder_[forkId]
                 << " and P" << philosopherId << " simultaneously!\n";
        }
        forkHeld_[forkId] = true;
        forkHolder_[forkId] = philosopherId;
    }
    LeaveCriticalSection(&cs_);
}

void InvariantChecker::checkAllForksReleased() {
    EnterCriticalSection(&cs_);
    for (int i = 0; i < numForks_; i++) {
        if (forkHeld_[i]) {
            violationCount_++;
            cout << "[INVARIANT VIOLATION] Fork " << i << " still held by P" << forkHolder_[i] << " after completion!\n";
        }
    }
    LeaveCriticalSection(&cs_);
}

void InvariantChecker::checkEatingRequiresBothForks(int philosopherId, bool hasLeft, bool hasRight, bool isEating) {
    if (isEating && (!hasLeft || !hasRight)) {
        violationCount_++;
        cout << "[INVARIANT VIOLATION] P" << philosopherId << " is eating without both forks! Left=" << hasLeft
             << " Right=" << hasRight << "\n";
    }
}