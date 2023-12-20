/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_CONNECTION_POOL_H
#define PROFILER_SERVER_CONNECTION_POOL_H

#include <string>
#include <memory>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <utility>
#include "ServerLog.h"
#include "TraceDatabase.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
class ConnectionPool {
public:
    explicit ConnectionPool(std::string dbPath, std::mutex &dbMutex);
    ~ConnectionPool();
    ConnectionPool(const ConnectionPool &) = delete;
    ConnectionPool &operator=(const ConnectionPool &) = delete;
    ConnectionPool(ConnectionPool &&) = delete;
    ConnectionPool &operator=(ConnectionPool &&) = delete;

    std::shared_ptr<TraceDatabase> GetConnection();
    void SetMaxActiveCount(int count);
    void SetMaxRetryCount(int count);
    void SetMaxWaitTime(int seconds);
    std::string GetDbPath();
    void Stop();

private:
    std::mutex mutex;
    std::mutex &insertSqlMutex;
    std::condition_variable cv;
    std::string path;
    bool valid = true;
    int maxActiveConnections = 10;
    int maxRetryAttempts = 3;
    int maxWaitTime = 2; // seconds
    std::deque<TraceDatabase*> idlePool;
    std::deque<TraceDatabase*> activePool;

    TraceDatabase* CreatConnection();
    void ReleaseConnection(TraceDatabase *conn);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_CONNECTION_POOL_H
