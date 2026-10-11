#pragma once

#include "mutex.h"

namespace rtos {
class LockGuard {
public:
    explicit LockGuard(Mutex& m, TickType_t timeout): mutex_(m), locked_(mutex_.lock(timeout)) {}
    ~LockGuard() { if (locked_) { mutex_.unlock(); }}
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;

    bool ownsLock() const { return locked_; }
private:
    Mutex& mutex_;
    bool locked_;
};

} //namespace rtos