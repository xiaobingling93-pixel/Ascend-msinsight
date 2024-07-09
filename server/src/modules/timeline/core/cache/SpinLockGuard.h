/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SPINLOCKGARD_H
#define PROFILER_SERVER_SPINLOCKGARD_H
#include <atomic>

namespace Dic::Module::Timeline {
class SpinLock {
public:
    SpinLock() : flag(ATOMIC_FLAG_INIT) {}

    void lock()
    {
        while (flag.test_and_set(std::memory_order_acquire)) {
            // 自旋等待，直到锁被释放
        }
    }

    void unlock()
    {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag;
};

class SpinLockGuard {
public:
    explicit SpinLockGuard(SpinLock &spinLock) : spinLock(spinLock)
    {
        spinLock.lock();
    }

    ~SpinLockGuard()
    {
        spinLock.unlock();
    }

    // 禁用复制和赋值
    SpinLockGuard(const SpinLockGuard &) = delete;
    SpinLockGuard &operator = (const SpinLockGuard &) = delete;

private:
    SpinLock &spinLock;
};
}


#endif // PROFILER_SERVER_SPINLOCKGARD_H
