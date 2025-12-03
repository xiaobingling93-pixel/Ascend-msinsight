/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_HTHREADPOOL_THREADPOOL_H
#define DATA_INSIGHT_CORE_HTHREADPOOL_THREADPOOL_H

#include <atomic>
#include <functional>
#include <future>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "SafeQueue.h"
#include "TraceIdManager.h"

namespace Dic {
class ThreadPool {
public:
    static ThreadPool& Instance();
    explicit ThreadPool(uint32_t threadCount);
    ~ThreadPool();
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    /**
     * Add a task to the queue.
     *
     * @param f: the task function
     * @param args: the task argument
     * @return the future of the result, future.get() throw exception of the task.
     */
    template<class F, class ...Args>
    auto AddTask(F &&f, const std::string &parentTraceId, Args &&...args) -> std::future<decltype(f(args...))>
    {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        std::function<void()> wrapperFunc = [task, parentTraceId]() {
            std::string curTraceId = TraceIdManager::GenerateTraceId();
            std::string traceId = parentTraceId == "" ? curTraceId : parentTraceId + "_" + curTraceId;
            TraceIdManager::SetTraceId(traceId);
            (*task)();
        };
        std::unique_lock lock(taskMutex);
        taskQueue.Push(wrapperFunc);
        taskCv.notify_one();
        return task->get_future();
    }

    /**
     * Wait for all tasks to complete.
     */
    void WaitForAllTasks();

    /**
     * Clear task queue, and wait for all running tasks to complete.
     */
    void Reset();

    /**
     * Wait for all running tasks to complete, and close the thread pool.
     */
    void ShutDown();

private:
    bool running = true;
    bool waiting = false;
    std::atomic<int> runningTasks{0};
    Dic::SafeQueue<std::function<void()>> taskQueue;
    std::vector<std::thread> threads;
    std::mutex taskMutex;
    std::condition_variable taskCv;
    std::condition_variable taskDoneCv;

    static void ThreadFunc(ThreadPool &threadPool, int index);
};
} // namespace Dic

#endif // DATA_INSIGHT_CORE_HTHREADPOOL_THREADPOOL_H
