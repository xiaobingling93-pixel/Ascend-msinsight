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
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
