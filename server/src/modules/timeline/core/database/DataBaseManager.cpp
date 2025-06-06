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

bool DataBaseManager::CreatConnectionPool(const std::string &rankId, const std::string &dbPath)
{
    const static unsigned int CPU_CORE_COUNT = SystemUtil::GetCpuCoreCount();
    std::unique_lock<std::recursive_mutex> lock(mutex);
    databasePathSet.emplace(dbPath);
    bool isBaseline = Global::BaselineManager::Instance().IsBaselineRankId(rankId);
    SetRankIdFileIdMapping(rankId, dbPath);
    DataType curDataType = isBaseline ? baselineType : dataType;
    std::string fileId = dbPath;
    if (traceDatabaseMap.count(fileId) == 0) {
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
        traceDatabaseMap.emplace(fileId, std::move(conn));
        return true;
    }
    ServerLog::Error("The file id has a connection. id:",
                     fileId,
                     ", old path:",
                     traceDatabaseMap.at(fileId)->GetDbPath(),
                     ", new path:",
                     dbPath);
    return false;
}

std::shared_ptr<VirtualTraceDatabase> DataBaseManager::GetTraceDatabaseByRankId(const std::string &rankId)
{
    std::string fileId = GetFileIdByRankId(rankId);
    return GetTraceDatabaseByFileId(fileId);
}

std::shared_ptr<VirtualTraceDatabase> DataBaseManager::GetTraceDatabaseByFileId(const std::string &fileId)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    auto it = traceDatabaseMap.find(fileId);
    if (it == traceDatabaseMap.end() && dbFilePathMap.count(fileId) != 0) {
        it = traceDatabaseMap.find(dbFilePathMap[fileId]);
    }
    if (it == traceDatabaseMap.end()) {
        ServerLog::Error("Can't find connection pool. fileId:", fileId);
        return nullptr;
    }
    auto ptr = it->second->GetConnection();
    return ptr;
}

std::string DataBaseManager::GetRankIdByFileId(const std::string &fileId)
{
    if (dataType == DataType::TEXT) {
        return fileId;
    }
    std::string hostRankStr;
    for (const auto &item: dbFilePathMap) {
        if (item.second == fileId) {
            hostRankStr = item.first;
            break;
        }
    }
    std::vector<std::string> hostRankVec = StringUtil::Split(hostRankStr, " ");
    // db场景下，格式为“host rankId”，因此按空格分割后取第二个
    const int expectSpliceSize = 2;
    if (hostRankVec.size() != expectSpliceSize) {
        return "";
    }
    return hostRankVec[1];
}

std::shared_ptr<Summary::VirtualSummaryDataBase> DataBaseManager::GetSummaryDatabaseByRankId(const std::string &rankId)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    std::vector<std::string> ids;
    std::shared_ptr<Summary::VirtualSummaryDataBase> db = nullptr;
    std::string fileId = GetFileIdByRankId(rankId);
    return GetSummaryDataBaseByFileId(fileId);
}

std::shared_ptr<Summary::VirtualSummaryDataBase> DataBaseManager::GetSummaryDataBaseByFileId(const std::string &fileId)
{
    std::unique_lock lock(mutex);
    auto it = summaryDatabaseMap.find(fileId);
    if (it == summaryDatabaseMap.end() && dbFilePathMap.count(fileId) != 0) {
        it = summaryDatabaseMap.find(dbFilePathMap[fileId]);
    }
    if (it == summaryDatabaseMap.end()) {
        ServerLog::Error("Can't find summary database. FileId:", fileId);
        return nullptr;
    }
    return it->second;
}

std::shared_ptr<Summary::VirtualSummaryDataBase> DataBaseManager::CreateSummaryDatabase(const std::string &rankId,
                                                                                        const std::string &dbPath)
{
    std::unique_lock lock(mutex);
    databasePathSet.emplace(dbPath);
    bool isBaseline = Global::BaselineManager::Instance().IsBaselineRankId(rankId);
    DataType curDataType = isBaseline ? baselineType : dataType;
    std::string fileId = dbPath;
    SetRankIdFileIdMapping(rankId, fileId);
    if (summaryDatabaseMap.count(fileId) == 0) {
        std::recursive_mutex &dbMutex = GetDbMutex(fileId);
        if (curDataType == DataType::TEXT) {
            summaryDatabaseMap.emplace(fileId, std::make_shared<Summary::TextSummaryDataBase>(dbMutex));
        } else if (curDataType == DataType::DB) {
            summaryDatabaseMap.emplace(fileId, std::make_shared<FullDb::DbSummaryDataBase>(dbMutex));
        }
    }
    return summaryDatabaseMap[fileId];
}

std::shared_ptr<Memory::VirtualMemoryDataBase> DataBaseManager::CreateMemoryDataBase(const std::string &rankId,
                                                                                     const std::string &dbPath)
{
    std::unique_lock lock(mutex);
    databasePathSet.emplace(dbPath);
    bool isBaseline = Global::BaselineManager::Instance().IsBaselineRankId(rankId);
    DataType curDataType = isBaseline ? baselineType : dataType;
    std::string fileId = dbPath;
    SetRankIdFileIdMapping(rankId, fileId);
    if (memoryDatabaseMap.count(fileId) == 0) {
        std::recursive_mutex &dbMutex = GetDbMutex(fileId);
        switch (curDataType) {
            case DataType::DB:
                memoryDatabaseMap.emplace(fileId, std::make_unique<FullDb::DbMemoryDataBase>(dbMutex));
                break;
            case DataType::TEXT:
            default:
                memoryDatabaseMap.emplace(fileId, std::make_unique<Memory::TextMemoryDataBase>(dbMutex));
                break;
        }
    }
    auto res = memoryDatabaseMap[fileId];
    if (!res->IsOpen()) {
        res->SetDbPath(dbPath);
    }
    return memoryDatabaseMap[fileId];
}

std::shared_ptr<Memory::VirtualMemoryDataBase> DataBaseManager::GetMemoryDatabaseByRankId(const std::string &rankId)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    std::string fileId = GetFileIdByRankId(rankId);
    return GetMemoryDatabaseByFileId(fileId);
}

std::shared_ptr<Memory::VirtualMemoryDataBase> DataBaseManager::GetMemoryDatabaseByFileId(const std::string &fileId)
{
    std::unique_lock lock(mutex);
    auto it = memoryDatabaseMap.find(fileId);
    if (it == memoryDatabaseMap.end() && dbFilePathMap.count(fileId) != 0) {
        it = memoryDatabaseMap.find(dbFilePathMap[fileId]);
    }
    if (it == memoryDatabaseMap.end()) {
        ServerLog::Error("Can't find memory database. FileId:", fileId);
        return nullptr;
    }
    return it->second;
}

void DataBaseManager::ReleaseDatabaseByRankId(const std::string &rankId)
{
    std::string fileId = GetFileIdByRankId(rankId);
    return ReleaseDatabaseByFileId(fileId);
}

void DataBaseManager::ReleaseDatabaseByFileId(const std::string &fileId)
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

bool DataBaseManager::HasRankId(DatabaseType type, const std::string &rankId)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    std::string fileId = GetFileIdByRankId(rankId);
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
    leaksMemoryDatabaseMap.clear();
    fileType = FileType::PYTORCH;
    rankId2FileIdMap.clear();
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

std::vector<std::shared_ptr<VirtualClusterDatabase>> DataBaseManager::GetAllClusterDatabase()
{
    std::vector<std::shared_ptr<VirtualClusterDatabase>> res;
    for (auto& [key, value]: clusterDatabaseMap) {
        res.push_back(value);
    }
    return res;
}

std::vector<std::string> DataBaseManager::GetAllRankId()
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    std::vector<std::string> traceFileId;
    for (auto &traceDatabase : traceDatabaseMap) {
        traceFileId.emplace_back(fileIdToRankIdMap[traceDatabase.first]);
    }
    return traceFileId;
}

std::string DataBaseManager::GetDbPathByRankId(const std::string &rankId)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    std::string fileId = GetFileIdByRankId(rankId);
    auto it = traceDatabaseMap.find(fileId);
    if (it == traceDatabaseMap.end()) {
        ServerLog::Error("Can't find db path for rank ", rankId);
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
        database = GetTraceDatabaseByRankId(id);
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
    if (!rankId.empty() && Global::BaselineManager::Instance().IsBaselineRankId(rankId)) {
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

void DataBaseManager::SetDbPathMapping(const std::string &rankId, const std::string &dbPath,
                                       const std::string &hostId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    rankId2FileIdMap[rankId] = dbPath;
    dbFilePathMap[rankId] = dbPath;
    host2DbPath[hostId].push_back(dbPath);
}

bool DataBaseManager::ResetBaseline()
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
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
    if (fileId.empty() && !leaksMemoryDatabaseMap.empty()) {
        return leaksMemoryDatabaseMap.begin()->second;
    }
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

std::string DataBaseManager::GetAnyTraceDatabaseId()
{
    if (traceDatabaseMap.empty()) {
        return "";
    }
    return traceDatabaseMap.begin()->first;
}
void DataBaseManager::SetRankIdFileIdMapping(const std::string &rankId, const std::string &fileId)
{
    rankId2FileIdMap[rankId] = fileId;
    fileIdToRankIdMap[fileId] = rankId;
}
std::string DataBaseManager::GetFileIdByRankId(const std::string &rankId) const
{
    if (rankId2FileIdMap.find(rankId) == rankId2FileIdMap.end()) {
        return "";
    }
    return rankId2FileIdMap.at(rankId);
}
void DataBaseManager::UpdateRankIdToDeviceId(const std::string &fileId,
                                             const std::string &rankId,
                                             const std::string &deviceId)
{
    if (fileId.empty() || rankId.empty()) {
        return;
    }
    std::string deviceIdTmp = deviceId;
    if (deviceId.empty()) {
        deviceIdTmp = rankId;
    }
    if (auto pos = rankId.find("Baseline_"); pos != std::string::npos) {
        deviceIdTmp = rankId.substr(pos + strlen("Baseline_"));
    }
    rankIdToDeviceIdMap[fileId + rankId] = deviceIdTmp;
}

std::string DataBaseManager::GetDeviceIdFromRankId(const std::string &rankId, const std::string& module)
{
    if (module == "memory") {
        std::string fileId = GetFileIdByRankId(rankId);
        return rankIdToDeviceIdMap[fileId + rankId];
    }
    if (module == "operator") {
        auto database = DataBaseManager::GetSummaryDatabaseByRankId(rankId);
        if (database == nullptr) {
            return "";
        }
        return database->QueryDeviceId();
    }
    return "";
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
