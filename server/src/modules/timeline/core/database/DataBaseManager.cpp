/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "DataBaseManager.h"
namespace Dic {
namespace Module {
namespace Timeline {
    using namespace Dic::Server;
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

Summary::SummaryDataBase *DataBaseManager::GetSummaryDatabase(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (summaryDatabaseMap.count(fileId) == 0) {
        summaryDatabaseMap.emplace(fileId, std::make_unique<Summary::SummaryDataBase>());
    }
    return summaryDatabaseMap[fileId].get();
}

Memory::MemoryDataBase *DataBaseManager::GetMemoryDatabase(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (memoryDatabaseMap.count(fileId) == 0) {
        memoryDatabaseMap.emplace(fileId, std::make_unique<Memory::MemoryDataBase>());
    }
    return memoryDatabaseMap[fileId].get();
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

std::vector<Memory::MemoryDataBase *> DataBaseManager::GetAllMemoryDatabase()
{
    std::unique_lock<std::mutex> lock(mutex);
    std::vector<Memory::MemoryDataBase *> databases;
    for (auto &databaseMap: memoryDatabaseMap) {
        databases.emplace_back(databaseMap.second.get());
    }
    return databases;
}

std::vector<Summary::SummaryDataBase *> DataBaseManager::GetAllSummaryDatabase()
{
    std::unique_lock<std::mutex> lock(mutex);
    std::vector<Summary::SummaryDataBase *> databases;
    for (auto &databaseMap: summaryDatabaseMap) {
        databases.emplace_back(databaseMap.second.get());
    }
    return databases;
}

bool DataBaseManager::MemoryHasFileId(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    return memoryDatabaseMap.count(fileId) != 0;
}

void DataBaseManager::Clear()
{
    std::unique_lock<std::mutex> lock(mutex);
    traceDatabaseMap.clear();
    memoryDatabaseMap.clear();
    summaryDatabaseMap.clear();
}

    ClusterDatabase *DataBaseManager::GetClusterDatabase()
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (clusterDatabaseMap.count("cluster") == 0) {
            clusterDatabaseMap.emplace("cluster", std::make_unique<ClusterDatabase>());
        }
        return clusterDatabaseMap["cluster"].get();
    }
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
