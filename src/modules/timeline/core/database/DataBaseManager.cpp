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

TraceDatabase *DataBaseManager::GetTraceDatabase(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (traceDatabaseMap.count(fileId) == 0) {
        traceDatabaseMap.emplace(fileId, std::make_unique<TraceDatabase>());
    }
    return traceDatabaseMap[fileId].get();
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

std::vector<TraceDatabase *> DataBaseManager::GetAllTraceDatabase()
{
    std::unique_lock<std::mutex> lock(mutex);
    std::vector<TraceDatabase *> traceDatabases;
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
