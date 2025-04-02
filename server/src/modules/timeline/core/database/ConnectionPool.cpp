/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */

#include "ConnectionPool.h"

namespace Dic::Module::Timeline {
ConnectionPool::ConnectionPool(std::string dbPath, std::function<VirtualTraceDatabase*()> call)
    : databaseCreateCall(std::move(call)), path(std::move(dbPath)) {}

ConnectionPool::~ConnectionPool()
{
    try {
        Stop();
    } catch (const std::exception &) {
        // do nothing
    }
}

std::shared_ptr<VirtualTraceDatabase> ConnectionPool::GetConnection()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (!valid) {
        return nullptr;
    }
    VirtualTraceDatabase *conn = nullptr;
    if (!idlePool.empty()) {
        conn = idlePool.front();
        idlePool.pop_front();
    } else if (activePool.size() < maxActiveConnections) {
        conn = CreatConnection();
    } else {
        ServerLog::Info("Wait idle connection.");
        cv.wait_for(lock, std::chrono::seconds(maxWaitTime));
        if (valid && !idlePool.empty()) {
            conn = idlePool.front();
            idlePool.pop_front();
        }
    }
    if (conn == nullptr) {
        ServerLog::Error("Get connection Failed.");
        return nullptr;
    }
    std::shared_ptr<VirtualTraceDatabase> connPtr(conn, [this] (VirtualTraceDatabase *conn) {
        ReleaseConnection(conn);
    });
    return connPtr;
}

/**
 * Set max active connections.
 *
 * @param count max active connections.
 */
void ConnectionPool::SetMaxActiveCount(unsigned int count)
{
    maxActiveConnections = std::max(maxActiveConnections, count);
}

void ConnectionPool::Stop()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (!valid) {
        return;
    }
    valid = false;
    ServerLog::Info("Wait all connection released. path:", path, ", idle size:", idlePool.size(),
                    ", active size:", activePool.size());
    while (idlePool.size() != activePool.size()) {
        cv.wait_for(lock, std::chrono::seconds(maxWaitTime));
    }
    ServerLog::Info("All connection released.");
    for (auto conn : activePool) {
        delete conn;
    }
    idlePool.clear();
    activePool.clear();
    path = "";
}

void ConnectionPool::ReleaseConnection(Timeline::VirtualTraceDatabase *conn)
{
    std::unique_lock<std::mutex> lock(mutex);
    idlePool.emplace_back(conn);
    if (valid) {
        cv.notify_one();
    } else {
        ServerLog::Info("Release connection. idle pool size:", idlePool.size());
        cv.notify_all();
    }
}

VirtualTraceDatabase *ConnectionPool::CreatConnection()
{
    int retryCount = 0;
    while (retryCount < maxRetryAttempts) {
        VirtualTraceDatabase *conn = databaseCreateCall();
        if (!conn->OpenDb(path, false)) {
            delete conn;
            retryCount++;
            continue;
        }
        activePool.emplace_back(conn);
        return conn;
    }
    ServerLog::Error("Failed create connect. path:", path);
    return nullptr;
}

std::string ConnectionPool::GetDbPath()
{
    return path;
}
} // end of namespace Timeline
// end of namespace Module
// end of namespace Dic