#ifndef FORK_H
#define FORK_H

#include <windows.h>
#include <string>

class Fork {
    public:
        Fork(int id);
        ~Fork();

        bool acquire(DWORD timeoutMs);
        void release();

        int getId() const { return id_; }
        bool isHeld() const { return held_; }
        int getHolderId() const { return holderId_; }

    private:
        int id_;
        HANDLE mutex_;
        bool held_;
        int holderId_;
};

#endif