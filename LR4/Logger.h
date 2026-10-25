#ifndef LOGGER_H
#define LOGGER_H

#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <chrono>

using namespace std;

struct LogEntry {
    DWORD timeMs;         
    int philosopherId;     
    string eventType; 
    string detail;  
};

class Logger {
    public:
        Logger();
        ~Logger();

        void init(bool enabled, bool verbose, const string& logFile = "experiment.log");
        void log(int philosopherId, const string& eventType, const string& detail = "");
        void flush();

        vector<LogEntry> getEntries() const;
        DWORD getStartTime() const;

    private:
        CRITICAL_SECTION cs_;
        bool enabled_;
        bool verbose_;
        ofstream file_;
        vector<LogEntry> entries_;
        DWORD startTime_;
        bool initialized_;
};

#endif