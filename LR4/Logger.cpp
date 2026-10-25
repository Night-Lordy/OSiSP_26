#include "Logger.h"
#include <iostream>
#include <sstream>

using namespace std;

Logger::Logger() : enabled_(false), verbose_(false), initialized_(false), startTime_(0) {
    InitializeCriticalSection(&cs_);
}

Logger::~Logger() {
    flush();
    if (file_.is_open()) file_.close();
    DeleteCriticalSection(&cs_);
}

void Logger::init(bool enabled, bool verbose, const string& logFile) {
    EnterCriticalSection(&cs_);
    enabled_ = enabled;
    verbose_ = verbose;
    startTime_ = GetTickCount();
    if (enabled_) {
        file_.open(logFile, ios::out | ios::trunc);
    }
    initialized_ = true;
    LeaveCriticalSection(&cs_);
}

void Logger::log(int philosopherId, const string& eventType, const string& detail) {
    if (!enabled_) return;

    EnterCriticalSection(&cs_);

    DWORD now = GetTickCount();
    DWORD elapsed = (now - startTime_);

    LogEntry entry;
    entry.timeMs = elapsed;
    entry.philosopherId = philosopherId;
    entry.eventType = eventType;
    entry.detail = detail;
    entries_.push_back(entry);

    if (verbose_) {
        ostringstream oss;
        oss << "[" << setw(6) << setfill('0') << elapsed << " ms] "
            << "P" << philosopherId << " " << eventType;
        if (!detail.empty()) oss << " " << detail;
        cout << oss.str() << endl;

        if (file_.is_open()) {
            file_ << oss.str() << endl;
        }
    }

    LeaveCriticalSection(&cs_);
}

void Logger::flush() {
    EnterCriticalSection(&cs_);
    if (file_.is_open()) file_.flush();
    LeaveCriticalSection(&cs_);
}

vector<LogEntry> Logger::getEntries() const {
    return entries_;
}

DWORD Logger::getStartTime() const {
    return startTime_;
}