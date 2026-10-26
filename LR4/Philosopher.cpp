#include "Philosopher.h"
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

Philosopher::Philosopher(const PhilosopherConfig& config,
    Fork* forks,
    Logger* logger,
    Stats* stats,
    InvariantChecker* invariants) : config_(config), forks_(forks), logger_(logger),
    stats_(stats), invariants_(invariants), state_(PhilosopherState::THINKING), threadHandle_(NULL), tid_(0) {}

Philosopher::~Philosopher() {
    if (threadHandle_ != NULL) {
        CloseHandle(threadHandle_);
    }
}

int Philosopher::getLeftFork() const {
    return config_.id;
}

int Philosopher::getRightFork() const {
    return (config_.id + 1) % config_.numForks;
}

void Philosopher::setState(PhilosopherState newState) {
    state_ = newState;

    if (newState == PhilosopherState::FINISHED) {
        std::cout << "[P" << config_.id << "] All cycles finished" << std::endl;
    }
    /*
    string stateStr;
    switch (newState) {
        case PhilosopherState::THINKING: stateStr = "THINKING"; break;
        case PhilosopherState::WAITING:  stateStr = "WAITING";  break;
        case PhilosopherState::EATING:   stateStr = "EATING";   break;
        case PhilosopherState::FINISHED: stateStr = "FINISHED"; break;
    }

    ostringstream oss;
    oss << "WORKER " << config_.id << " | TID=" << tid_ << " | STATE=" << stateStr;

    if (logger_) {
        logger_->log(config_.id, stateStr);
    }

    cout << oss.str() << endl;
    */
}

DWORD WINAPI Philosopher::threadProc(LPVOID param) {
    Philosopher* self = static_cast<Philosopher*>(param);
    self->run();
    return 0;
}

void Philosopher::run() {
    tid_ = GetCurrentThreadId();

    int leftFork = getLeftFork();
    int rightFork = getRightFork();

    bool evenPhilosopher = (config_.id % 2 == 0);

    int firstFork, secondFork;
    if (evenPhilosopher) {
        firstFork = leftFork;
        secondFork = rightFork;
    }
    else {
        firstFork = rightFork;
        secondFork = leftFork;
    }

    for (int cycle = 0; cycle < config_.numCycles; cycle++) {

        setState(PhilosopherState::THINKING);
        logger_->log(config_.id, "THINKING", "cycle=" + to_string(cycle + 1));
        Sleep(config_.thinkingTimeMs);

        setState(PhilosopherState::WAITING);
        logger_->log(config_.id, "WAITING", "RESOURCE=" + to_string(firstFork));

        bool acquired = forks_[firstFork].acquire(config_.maxWaitTimeMs);
        if (!acquired) {
            logger_->log(config_.id, "TIMEOUT", "RESOURCE=" + to_string(firstFork));
            stats_->recordCycle(config_.id, config_.thinkingTimeMs, 0, 0, true);
            continue; 
        }
        invariants_->checkForkExclusivity(firstFork, config_.id);
        logger_->log(config_.id, "ACQUIRED", "RESOURCE=" + to_string(firstFork));

        logger_->log(config_.id, "WAITING", "RESOURCE=" + to_string(secondFork));

        acquired = forks_[secondFork].acquire(config_.maxWaitTimeMs);
        if (!acquired) {
            forks_[firstFork].release();
            logger_->log(config_.id, "TIMEOUT", "RESOURCE=" + to_string(secondFork));
            stats_->recordCycle(config_.id, config_.thinkingTimeMs, 0, 0, true);
            continue;
        }
        invariants_->checkForkExclusivity(secondFork, config_.id);
        logger_->log(config_.id, "ACQUIRED", "RESOURCE=" + to_string(secondFork));

        setState(PhilosopherState::EATING);
        invariants_->checkEatingRequiresBothForks(config_.id, true, true, true);
        logger_->log(config_.id, "WORKING");
        Sleep(config_.eatingTimeMs);

        forks_[firstFork].release();
        logger_->log(config_.id, "RELEASED", "RESOURCE=" + to_string(firstFork));

        forks_[secondFork].release();
        logger_->log(config_.id, "RELEASED", "RESOURCE=" + to_string(secondFork));

        stats_->recordCycle(config_.id, config_.thinkingTimeMs, config_.eatingTimeMs, 0, false);
    }

    setState(PhilosopherState::FINISHED);
    logger_->log(config_.id, "FINISHED");
}