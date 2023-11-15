/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "DataBaseManager.h"
namespace Dic {
namespace Module {
namespace Timeline {

DataBaseManager &DataBaseManager::Instance()
{
    static DataBaseManager instance;
    return instance;
}

bool DataBaseManager::CreatConnectionPool(const std::string &fileId, const std::string &dbPath)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (traceDatabaseMap.count(fileId) == 0) {
        traceDatabaseMap.emplace(fileId, std::make_unique<ConnectionPool>(dbPath));
        return true;
    }
    ServerLog::Error("The file id has a connection. id:", fileId, ", old path:",
                     traceDatabaseMap.at(fileId)->GetDbPath(), ", new path:", dbPath);
    return false;
}

std::shared_ptr<TraceDatabase> DataBaseManager::GetTraceDatabase(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (traceDatabaseMap.count(fileId) == 0) {
        return nullptr;
    }
    return traceDatabaseMap[fileId]->GetConnection();
}

void DataBaseManager::ReleaseTraceDatabase(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (traceDatabaseMap.count(fileId) >= 0) {
        traceDatabaseMap.erase(fileId);
    }
}

bool DataBaseManager::HasFileId(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    return traceDatabaseMap.count(fileId) != 0;
}

std::vector<ConnectionPool *> DataBaseManager::GetAllTraceDatabase()
{
    std::unique_lock<std::mutex> lock(mutex);
    std::vector<ConnectionPool *> traceDatabases;
    for (auto &traceDatabase : traceDatabaseMap) {
        traceDatabases.emplace_back(traceDatabase.second.get());
    }
    return traceDatabases;
}

void DataBaseManager::Clear()
{
    std::unique_lock<std::mutex> lock(mutex);
    traceDatabaseMap.clear();
}

std::vector<std::string> DataBaseManager::GetAllFileId()
{
    std::unique_lock<std::mutex> lock(mutex);
    std::vector<std::string> traceFileId;
    for (auto &traceDatabase : traceDatabaseMap) {
        traceFileId.emplace_back(traceDatabase.first);
    }
    return traceFileId;
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
