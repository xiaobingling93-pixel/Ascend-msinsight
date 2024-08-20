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
    std::unique_lock<std::mutex> lock(mutex);
    bool isBaseline = Global::BaselineManager::IsBaselineId(fileId);
    std::map<std::string, std::unique_ptr<ConnectionPool>> &curTraceDbMap =
            isBaseline ? traceBaselineDatabaseMap : traceDatabaseMap;
    DataType curDataType = isBaseline ? baselineType : dataType;
    if (curTraceDbMap.count(fileId) == 0) {
        std::recursive_mutex &dbMutex = GetDbMutex(fileId);
        std::unique_ptr<ConnectionPool> conn;
        switch (curDataType) {
            case DataType::DB:
                conn = std::make_unique<ConnectionPool>(dbPath, [&dbMutex]() {
                    return new FullDb::DbTraceDataBase(dbMutex);
                });
                break;
            case DataType::TEXT:
            default:
                conn = std::make_unique<ConnectionPool>(dbPath, [&dbMutex]() {
                    return new TextTraceDatabase(dbMutex);
                });
                break;
        }
        conn->SetMaxActiveCount(CPU_CORE_COUNT);
        curTraceDbMap.emplace(fileId, std::move(conn));
        return true;
    }
    ServerLog::Error("The file id has a connection. id:", fileId, ", old path:",
                     curTraceDbMap.at(fileId)->GetDbPath(), ", new path:", dbPath);
    return false;
}

std::shared_ptr<VirtualTraceDatabase> DataBaseManager::GetTraceDatabase(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    bool isBaseline = Global::BaselineManager::IsBaselineId(fileId);
    std::map<std::string, std::unique_ptr<ConnectionPool>> &curTraceDbMap =
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

Summary::VirtualSummaryDataBase *DataBaseManager::GetSummaryDatabase(const std::string &inputId)
{
    std::unique_lock<std::mutex> lock(mutex);
    bool isBaseline = Global::BaselineManager::IsBaselineId(inputId);
    // 获取当前map，如果fileId包含baseline字样则从summaryBaselineDatabaseMap中获取，否则从summaryDatabaseMap获取
    std::map<std::string, std::unique_ptr<Summary::VirtualSummaryDataBase>> &curSummaryDbMap =
            isBaseline ? summaryBaselineDatabaseMap : summaryDatabaseMap;
    DataType curDataType = isBaseline ? baselineType : dataType;
    std::string fileId = inputId;
    if (curSummaryDbMap.count(fileId) == 0) {
        std::recursive_mutex &dbMutex = GetDbMutex(fileId);
        if (curDataType == DataType::TEXT) {
            curSummaryDbMap.emplace(fileId, std::make_unique<Summary::TextSummaryDataBase>(dbMutex));
        } else if (curDataType == DataType::DB) {
            curSummaryDbMap.emplace(fileId, std::make_unique<FullDb::DbSummaryDataBase>(dbMutex));
        }
    }
    return curSummaryDbMap[fileId].get();
}

Memory::VirtualMemoryDataBase *DataBaseManager::GetMemoryDatabase(const std::string &inputId)
{
    std::unique_lock<std::mutex> lock(mutex);
    bool isBaseline = Global::BaselineManager::IsBaselineId(inputId);
    // 获取当前map，如果fileId包含baseline字样则从summaryBaselineDatabaseMap中获取，否则从summaryDatabaseMap获取
    std::map<std::string, std::unique_ptr<Memory::VirtualMemoryDataBase>> &curMemoryDbMap =
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
    return curMemoryDbMap[fileId].get();
}

void DataBaseManager::ReleaseDatabase(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
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
    std::unique_lock<std::mutex> lock(mutex);
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
    std::unique_lock<std::mutex> lock(mutex);
    std::vector<ConnectionPool *> traceDatabases;
    for (auto &traceDatabase : traceDatabaseMap) {
        traceDatabases.emplace_back(traceDatabase.second.get());
    }
    return traceDatabases;
}

std::vector<Memory::VirtualMemoryDataBase *> DataBaseManager::GetAllMemoryDatabase()
{
    std::unique_lock<std::mutex> lock(mutex);
    std::vector<Memory::VirtualMemoryDataBase *> databases;
    for (auto &databaseMap: memoryDatabaseMap) {
        databases.emplace_back(databaseMap.second.get());
    }
    return databases;
}

std::vector<Summary::VirtualSummaryDataBase *> DataBaseManager::GetAllSummaryDatabase()
{
    std::unique_lock<std::mutex> lock(mutex);
    std::vector<Summary::VirtualSummaryDataBase *> databases;
    for (auto &databaseMap: summaryDatabaseMap) {
        databases.emplace_back(databaseMap.second.get());
    }
    return databases;
}

void DataBaseManager::Clear()
{
    std::unique_lock<std::mutex> lock(mutex);
    traceDatabaseMap.clear();
    memoryDatabaseMap.clear();
    summaryDatabaseMap.clear();
    clusterDatabaseMap.clear();
    dbMutexMap.clear();
    dbFilePathMap.clear();
    host2DbPath.clear();
}

void DataBaseManager::Clear(DatabaseType type)
{
    std::unique_lock<std::mutex> lock(mutex);
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
        default:
            break;
    }
}

void DataBaseManager::ClearClusterDb()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (clusterDatabaseMap.count("cluster_w") != 0) {
        clusterDatabaseMap["cluster_w"].get()->CloseDb();
    }
    if (clusterDatabaseMap.count("cluster_r") != 0) {
        clusterDatabaseMap["cluster_r"].get()->CloseDb();
    }
    clusterDatabaseMap.clear();
}

VirtualClusterDatabase *DataBaseManager::GetWriteClusterDatabase()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (clusterDatabaseMap.count("cluster_w") == 0) {
        if (dataType == DataType::TEXT) {
            clusterDatabaseMap.emplace("cluster_w", std::make_unique<TextClusterDatabase>(GetDbMutex("cluster_w")));
        } else if (dataType == DataType::DB) {
            clusterDatabaseMap.emplace("cluster_w", std::make_unique<DbClusterDataBase>(GetDbMutex("cluster_w")));
        }
    }
    return clusterDatabaseMap["cluster_w"].get();
}

VirtualClusterDatabase *DataBaseManager::GetReadClusterDatabase()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (clusterDatabaseMap.count("cluster_r") == 0) {
        if (dataType == DataType::TEXT) {
            clusterDatabaseMap.emplace("cluster_r", std::make_unique<TextClusterDatabase>(GetDbMutex("cluster_r")));
        } else if (dataType == DataType::DB) {
            clusterDatabaseMap.emplace("cluster_r", std::make_unique<DbClusterDataBase>(GetDbMutex("cluster_r")));
        }
    }
    return clusterDatabaseMap["cluster_r"].get();
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

std::string DataBaseManager::GetDbPath(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
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

std::shared_ptr<VirtualTraceDatabase> DataBaseManager::GetTraceDatabaseWithOutHost(const std::string &fileId)
{
    std::vector<std::string> ids;
    for (const auto &hostInfo: host2DbPath) {
        ids.push_back(StringUtil::ReplaceFirst(hostInfo.first, "Host", fileId));
    }
    std::shared_ptr<VirtualTraceDatabase> database;
    for (const auto &id: ids) {
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
    if (!rankId.empty() && Global::BaselineManager::IsBaselineId(rankId)) {
        return baselineFileType;
    }
    return fileType;
}
void DataBaseManager::SetFileType(FileType type)
{
    std::lock_guard<std::mutex> lock(mutex);
    fileType = type;
}

void DataBaseManager::SetBaselineFileType(FileType type)
{
    std::lock_guard<std::mutex> lock(mutex);
    baselineFileType = type;
}

void DataBaseManager::SetBaselineDataType(DataType type)
{
    std::lock_guard<std::mutex> lock(mutex);
    baselineType = type;
}

void DataBaseManager::SetDbPathMapping(const std::string &rankId, const std::string &filePath,
                                       const std::string& hostId)
{
    std::lock_guard<std::mutex> lock(mutex);
    dbFilePathMap[rankId] = filePath;
    host2DbPath[hostId].push_back(filePath);
}

bool DataBaseManager::ResetBaseline()
{
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto &item: traceBaselineDatabaseMap) {
        if (item.second != nullptr) {
            item.second->Stop();
        }
    }
    for (const auto &item: memoryBaselineDatabaseMap) {
        if (item.second != nullptr) {
            item.second->CloseDb();
        }
    }
    memoryBaselineDatabaseMap.clear();
    for (auto &item: summaryBaselineDatabaseMap) {
        if (item.second != nullptr) {
            item.second->CloseDb();
        }
    }
    return false;
}

} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
