#include "Fork.h"
#include <iostream>

using namespace std;

Fork::Fork(int id) : id_(id), held_(false), holderId_(-1) {
    string name = "ForkMutex_" + to_string(id);
    mutex_ = CreateMutexA(NULL, FALSE, name.c_str());
    if (mutex_ == NULL) {
        cout << "Error creating mutex for fork " << id << ": " << GetLastError() << endl;
    }
}

Fork::~Fork() {
    if (mutex_ != NULL) {
        CloseHandle(mutex_);
    }
}

bool Fork::acquire(DWORD timeoutMs) {
    DWORD result = WaitForSingleObject(mutex_, timeoutMs);
    if (result == WAIT_OBJECT_0) {
        held_ = true;
        return true;
    }
    return false;
}

void Fork::release() {
    if (held_) {
        held_ = false;
        holderId_ = -1;
        ReleaseMutex(mutex_);
    }
}