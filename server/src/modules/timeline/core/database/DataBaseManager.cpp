/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */

#include "pch.h"
#include "TextMemoryDataBase.h"
#include "DbTraceDataBase.h"
#include "DbMemoryDataBase.h"
#include "DbSummaryDataBase.h"
#include "DbClusterDataBase.h"
#include "BaselineManager.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace FullDb;
DataBaseManager &DataBaseManager::Instance()
{
    static DataBaseManager instance;
    return instance;
}

bool DataBaseManager::CreatConnectionPool(const std::string &fileId, const std::string &dbPath)
{
    const static unsigned int CPU_CORE_COUNT = SystemUtil::GetCpuCoreCount();
    std::unique_lock<std::recursive_mutex> lock(mutex);
    databasePathSet.emplace(dbPath);
    bool isBaseline = Global::BaselineManager::Instance().IsBaselineId(fileId);
    std::map<std::string, std::shared_ptr<ConnectionPool>> &curTraceDbMap =
        isBaseline ? traceBaselineDatabaseMap : traceDatabaseMap;
    DataType curDataType = isBaseline ? baselineType : dataType;
    if (curTraceDbMap.count(fileId) == 0) {
        std::recursive_mutex &dbMutex = GetDbMutex(fileId);
        std::shared_ptr<ConnectionPool> conn;
        switch (curDataType) {
            case DataType::DB:
                conn = std::make_shared<ConnectionPool>(dbPath,
                    [&dbMutex]() { return new FullDb::DbTraceDataBase(dbMutex); });
                break;
            case DataType::TEXT:
            default:
                conn =
                    std::make_shared<ConnectionPool>(dbPath, [&dbMutex]() { return new TextTraceDatabase(dbMutex); });
                break;
        }
        conn->SetMaxActiveCount(CPU_CORE_COUNT);
        curTraceDbMap.emplace(fileId, std::move(conn));
        return true;
    }
    ServerLog::Error("The file id has a connection. id:", fileId, ", old path:", curTraceDbMap.at(fileId)->GetDbPath(),
        ", new path:", dbPath);
    return false;
}

std::shared_ptr<VirtualTraceDatabase> DataBaseManager::GetTraceDatabase(const std::string &fileId)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    bool isBaseline = Global::BaselineManager::Instance().IsBaselineId(fileId);
    std::map<std::string, std::shared_ptr<ConnectionPool>> &curTraceDbMap =
        isBaseline ? traceBaselineDatabaseMap : traceDatabaseMap;
    auto it = curTraceDbMap.find(fileId);
    if (it == curTraceDbMap.end() && dbFilePathMap.count(fileId) != 0) {
        it = curTraceDbMap.find(dbFilePathMap[fileId]);
    }
    if (it == curTraceDbMap.end()) {
        ServerLog::Error("Can't find connection pool. fileId:", fileId);
        return nullptr;
    }
    return it->second->GetConnection();
}

std::shared_ptr<Summary::VirtualSummaryDataBase> DataBaseManager::GetSummaryDatabase(const std::string &inputId)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    bool isBaseline = Global::BaselineManager::Instance().IsBaselineId(inputId);
    // 获取当前map，如果fileId包含baseline字样则从summaryBaselineDatabaseMap中获取，否则从summaryDatabaseMap获取
    std::map<std::string, std::shared_ptr<Summary::VirtualSummaryDataBase>> &curSummaryDbMap =
        isBaseline ? summaryBaselineDatabaseMap : summaryDatabaseMap;
    DataType curDataType = isBaseline ? baselineType : dataType;
    auto func = [&curSummaryDbMap, this, &curDataType](const std::string &inputId) ->
        std::shared_ptr<Summary::VirtualSummaryDataBase> {
        if (curSummaryDbMap.count(inputId) == 0) {
            std::recursive_mutex &dbMutex = GetDbMutex(inputId);
            if (curDataType == DataType::TEXT) {
                curSummaryDbMap.emplace(inputId, std::make_shared<Summary::TextSummaryDataBase>(dbMutex));
            } else if (curDataType == DataType::DB) {
                curSummaryDbMap.emplace(inputId, std::make_shared<FullDb::DbSummaryDataBase>(dbMutex));
            }
        }
        return curSummaryDbMap[inputId];
    };
    std::shared_ptr<Summary::VirtualSummaryDataBase> db;
    std::vector<std::string> ids;
    for (const auto &hostInfo : host2DbPath) {
        auto host = StringUtil::ReplaceFirst(hostInfo.first, "Host", "");
        if (!StringUtil::StartWith(inputId, host)) {
            ids.push_back(StringUtil::ReplaceFirst(hostInfo.first, "Host", inputId));
        }
    }
    for (const auto &id : ids) {
        db = func(id);
        if (db != nullptr) {
            break;
        }
    }
    if (db == nullptr) {
        db = func(inputId);
    }
    return db;
}

std::shared_ptr<Memory::VirtualMemoryDataBase> DataBaseManager::GetMemoryDatabase(const std::string &inputId)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    bool isBaseline = Global::BaselineManager::Instance().IsBaselineId(inputId);
    // 获取当前map，如果fileId包含baseline字样则从memoryBaselineDatabaseMap中获取，否则从memoryDatabaseMap获取
    std::map<std::string, std::shared_ptr<Memory::VirtualMemoryDataBase>> &curMemoryDbMap =
        isBaseline ? memoryBaselineDatabaseMap : memoryDatabaseMap;
    DataType curDataType = isBaseline ? baselineType : dataType;
    std::string fileId = inputId;
    if (curMemoryDbMap.count(fileId) == 0) {
        std::recursive_mutex &dbMutex = GetDbMutex(fileId);
        switch (curDataType) {
            case DataType::DB:
                curMemoryDbMap.emplace(fileId, std::make_unique<FullDb::DbMemoryDataBase>(dbMutex));
                break;
            case DataType::TEXT:
            default:
                curMemoryDbMap.emplace(fileId, std::make_unique<Memory::TextMemoryDataBase>(dbMutex));
                break;
        }
    }
    return curMemoryDbMap[fileId];
}

void DataBaseManager::ReleaseDatabase(const std::string &fileId)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    auto traceDataBase = traceDatabaseMap.find(fileId);
    if (traceDataBase != traceDatabaseMap.end()) {
        traceDatabaseMap.erase(traceDataBase);
    }
    auto memory = memoryDatabaseMap.find(fileId);
    if (memory != memoryDatabaseMap.end()) {
        memoryDatabaseMap.erase(memory);
    }
    auto summary = summaryDatabaseMap.find(fileId);
    if (summary != summaryDatabaseMap.end()) {
        summaryDatabaseMap.erase(summary);
    }
}

bool DataBaseManager::HasFileId(DatabaseType type, const std::string &fileId)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    bool result = false;
    switch (type) {
        case DatabaseType::TRACE:
            result = traceDatabaseMap.find(fileId) != traceDatabaseMap.end();
            break;
        case DatabaseType::SUMMARY:
            result = summaryDatabaseMap.find(fileId) != summaryDatabaseMap.end();
            break;
        case DatabaseType::MEMORY:
            result = memoryDatabaseMap.find(fileId) != memoryDatabaseMap.end();
            break;
        default:
            break;
    }
    return result;
}

std::vector<ConnectionPool *> DataBaseManager::GetAllTraceDatabase()
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    std::vector<ConnectionPool *> traceDatabases;
    for (auto &traceDatabase : traceDatabaseMap) {
        traceDatabases.emplace_back(traceDatabase.second.get());
    }
    return traceDatabases;
}

std::vector<Memory::VirtualMemoryDataBase *> DataBaseManager::GetAllMemoryDatabase()
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    std::vector<Memory::VirtualMemoryDataBase *> databases;
    for (auto &databaseMap : memoryDatabaseMap) {
        databases.emplace_back(databaseMap.second.get());
    }
    return databases;
}

std::vector<Summary::VirtualSummaryDataBase *> DataBaseManager::GetAllSummaryDatabase()
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    std::vector<Summary::VirtualSummaryDataBase *> databases;
    for (auto &databaseMap : summaryDatabaseMap) {
        databases.emplace_back(databaseMap.second.get());
    }
    return databases;
}

void DataBaseManager::Clear()
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    traceDatabaseMap.clear();
    memoryDatabaseMap.clear();
    summaryDatabaseMap.clear();
    clusterDatabaseMap.clear();
    dbMutexMap.clear();
    dbFilePathMap.clear();
    host2DbPath.clear();
    databasePathSet.clear();
}

void DataBaseManager::Clear(DatabaseType type)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    switch (type) {
        case DatabaseType::TRACE:
            traceDatabaseMap.clear();
            break;
        case DatabaseType::SUMMARY:
            summaryDatabaseMap.clear();
            break;
        case DatabaseType::MEMORY:
            memoryDatabaseMap.clear();
            break;
        case DatabaseType::LEAKS:
            leaksMemoryDatabaseMap.clear();
            break;
        default:
            break;
    }
}

void DataBaseManager::EraseClusterDb(const std::string &uniqueKey)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (clusterDatabaseMap.count(uniqueKey) != 0) {
        clusterDatabaseMap[uniqueKey].get()->CloseDb();
        clusterDatabaseMap.erase(clusterDatabaseMap.find(uniqueKey));
    }
}

void DataBaseManager::ClearClusterDb()
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    for (const auto &item: clusterDatabaseMap) {
        if (item.second != nullptr) {
            item.second->ReleaseStmt();
            item.second->CloseDb();
        }
    }
    clusterDatabaseMap.clear();
}

/**
 * 创建集群db对象（如果对象已存在，则清除重新创建）
 *
 * @param uniqueKey 唯一键
 * @param type db类型
 * @return
 */
std::shared_ptr<VirtualClusterDatabase> DataBaseManager::CreateClusterDatabase(const std::string &uniqueKey,
                                                                               DataType type)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (clusterDatabaseMap.count(uniqueKey) != 0) {
        EraseClusterDb(uniqueKey);
    }
    if (type == DataType::TEXT) {
        clusterDatabaseMap.emplace(uniqueKey, std::make_shared<TextClusterDatabase>(GetDbMutex(uniqueKey)));
    } else if (type == DataType::DB) {
        clusterDatabaseMap.emplace(uniqueKey, std::make_shared<DbClusterDataBase>(GetDbMutex(uniqueKey)));
    }
    return clusterDatabaseMap[uniqueKey];
}

/**
 * 根据唯一键获取集群db（会返回空指针，如不存在会返回空指针，外层需做好空指针校验）
 *
 * @param uniqueKey 唯一键
 * @return
 */
std::shared_ptr<VirtualClusterDatabase> DataBaseManager::GetClusterDatabase(const std::string &uniqueKey)
{
    if (clusterDatabaseMap.count(uniqueKey) == 0) {
        return nullptr;
    }
    return clusterDatabaseMap[uniqueKey];
}

std::vector<std::string> DataBaseManager::GetAllFileId()
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    std::vector<std::string> traceFileId;
    for (auto &traceDatabase : traceDatabaseMap) {
        traceFileId.emplace_back(traceDatabase.first);
    }
    return traceFileId;
}

std::string DataBaseManager::GetDbPath(const std::string &fileId)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    auto it = traceDatabaseMap.find(fileId);
    if (it == traceDatabaseMap.end() && dbFilePathMap.count(fileId) != 0) {
        it = traceDatabaseMap.find(dbFilePathMap[fileId]);
    }
    if (it == traceDatabaseMap.end()) {
        ServerLog::Error("Can't find db path for rank ", fileId);
        return "";
    }
    return it->second->GetDbPath();
}

std::shared_ptr<VirtualTraceDatabase> DataBaseManager::GetTraceDatabaseWithOutHost(const std::string &rankId)
{
    std::vector<std::string> ids;
    for (const auto &hostInfo : host2DbPath) {
        ids.push_back(StringUtil::ReplaceFirst(hostInfo.first, "Host", rankId));
    }
    std::shared_ptr<VirtualTraceDatabase> database;
    for (const auto &id : ids) {
        database = GetTraceDatabase(id);
        if (database != nullptr) {
            break;
        }
    }
    return database;
}

std::recursive_mutex &DataBaseManager::GetDbMutex(const std::string &fileId)
{
    return dbMutexMap[fileId];
}

DataType DataBaseManager::GetDataType()
{
    return dataType;
}
void DataBaseManager::SetDataType(DataType type)
{
    dataType = type;
}

FileType DataBaseManager::GetFileType()
{
    return fileType;
}

FileType DataBaseManager::GetFileTypeByRankId(const std::string &rankId)
{
    if (!rankId.empty() && Global::BaselineManager::Instance().IsBaselineId(rankId)) {
        return baselineFileType;
    }
    return fileType;
}
void DataBaseManager::SetFileType(FileType type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    fileType = type;
}

void DataBaseManager::SetBaselineFileType(FileType type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    baselineFileType = type;
}

void DataBaseManager::SetBaselineDataType(DataType type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    baselineType = type;
}

void DataBaseManager::SetDbPathMapping(const std::string &fileId, const std::string &filePath,
    const std::string &hostId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    dbFilePathMap[fileId] = filePath;
    host2DbPath[hostId].push_back(filePath);
}

bool DataBaseManager::ResetBaseline()
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    for (const auto &item : traceBaselineDatabaseMap) {
        if (item.second != nullptr) {
            std::string databasePath = item.second->GetDbPath();
            databasePathSet.erase(databasePath);
            item.second->Stop();
        }
    }
    traceBaselineDatabaseMap.clear();
    for (const auto &item : memoryBaselineDatabaseMap) {
        if (item.second != nullptr) {
            item.second->CloseDb();
        }
    }
    memoryBaselineDatabaseMap.clear();
    for (auto &item : summaryBaselineDatabaseMap) {
        if (item.second != nullptr) {
            item.second->CloseDb();
        }
    }
    EraseClusterDb(BASELINE);
    summaryBaselineDatabaseMap.clear();
    return false;
}

bool DataBaseManager::IsContainDatabasePath(const std::string &databasePath)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    return (databasePathSet.count(databasePath) > 0);
}

std::shared_ptr<FullDb::LeaksMemoryDatabase> DataBaseManager::GetLeaksMemoryDatabase(const std::string &fileId)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (leaksMemoryDatabaseMap.count(fileId) == 0) {
        std::recursive_mutex &dbMutex = GetDbMutex(fileId);
        leaksMemoryDatabaseMap.emplace(fileId, std::make_unique<FullDb::LeaksMemoryDatabase>(dbMutex));
    }
    return leaksMemoryDatabaseMap[fileId];
}

std::vector<FullDb::LeaksMemoryDatabase *> DataBaseManager::GetAllLeaksMemoryDatabase()
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    std::vector<FullDb::LeaksMemoryDatabase *> leaksDatabases;
    for (auto &leaksDatabase : leaksMemoryDatabaseMap) {
        leaksDatabases.emplace_back(leaksDatabase.second.get());
    }
    return leaksDatabases;
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
