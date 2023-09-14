/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef THREADPOOL_SAFEQUEUE_H
#define THREADPOOL_SAFEQUEUE_H
#include <deque>
#include <mutex>

namespace Dic {
template <typename T>
class SafeQueue {
public:
    void Push(T &t)
    {
        std::unique_lock<std::mutex> lock(mutex);
        deque.emplace_back(t);
    }

    bool Pop(T &t)
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (deque.empty()) {
            return false;
        }
        t = std::move(deque.front());
        deque.pop_front();
        return true;
    }

    bool Empty()
    {
        std::unique_lock<std::mutex> lock(mutex);
        return deque.empty();
    }

    size_t Size()
    {
        std::unique_lock<std::mutex> lock(mutex);
        return deque.size();
    }

    void Clear()
    {
        std::unique_lock<std::mutex> lock(mutex);
        deque.clear();
    }

private:
    std::deque<T> deque;
    std::mutex mutex;
};
}

#endif // THREADPOOL_SAFEQUEUE_H