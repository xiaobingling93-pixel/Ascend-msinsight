/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "WsSender.h"
#include "ParserStatusManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "DbTraceDataBase.h"
#include "DbMemoryDataBase.h"
#include "DbSummaryDataBase.h"
#include "ClusterParseThreadPoolExecutor.h"
#include "BaselineManager.h"
#include "CollectionTimeService.h"
#include "LeaksMemoryDatabase.h"
#include "LeaksMemoryService.h"
#include "TrackInfoManager.h"
#include "CacheManager.h"
#include "ParseUnitManager.h"
#include "FullDbParser.h"

namespace Dic::Module::FullDb {
using namespace Dic::Server;
using namespace Dic::Protocol;

FullDbParser &FullDbParser::Instance()
{
    static FullDbParser instance;
    return instance;
}

FullDbParser::FullDbParser()
{
    threadPool = std::make_unique<ThreadPool>(maxThreadNum);
}

FullDbParser::~FullDbParser()
{
    threadPool->ShutDown();
}

bool FullDbParser::Parse(const std::vector<std::string> &rankIds, const std::string &fileId)
{
    ServerLog::Info("start db parse.");
    for (auto id:rankIds) {
        Timeline::ParserStatusManager::Instance().SetParserStatus(id, Timeline::ParserStatus::INIT);
    }
    auto &instance = FullDbParser::Instance();
    instance.threadPool->AddTask(InitOpenDb, TraceIdManager::GetTraceId(), fileId, rankIds);
    return true;
}

void FullDbParser::Reset()
{
    ServerLog::Info("Reset. wait task completed.");
    Timeline::ParserStatusManager::Instance().SetAllTerminateStatus();
    threadPool->Reset();
    ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->Reset();
    ServerLog::Info("Task completed.");
    auto connList = Timeline::DataBaseManager::Instance().GetAllTraceDatabase();
    for (auto &conn : connList) {
        conn->Stop();
    }
    TrackInfoManager::Instance().Reset();
    CacheManager::Instance().ClearAll();
    DataBaseManager::Instance().ClearClusterDb();
    Timeline::DataBaseManager::Instance().Clear();
    Timeline::TraceTime::Instance().Reset();
    FileParser::Reset();
    Timeline::ParserStatusManager::Instance().ClearAllParserStatus();
    FullDb::DbMemoryDataBase::Reset();
    FullDb::DbSummaryDataBase::Reset();
    FullDb::DbTraceDataBase::Reset();
    FullDb::LeaksMemoryDatabase::Reset();
    ServerLog::Info("End Reset trace Parser");
    CollectionTimeService::Instance().Reset();
}

// 此方法为私有方法，调用前需保证不会出现空指针的情况
std::shared_ptr<DbTraceDataBase> FullDbParser::GetTraceDatabase(const std::string &filePath)
{
    auto db = Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId(filePath);
    return std::dynamic_pointer_cast<DbTraceDataBase, Timeline::VirtualTraceDatabase>(db);
}

void FullDbParser::InitOpenDb(const std::string &filePath, const std::vector<std::string> &rankIds)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::string dbId = (rankIds.size() > 0 && Global::BaselineManager::Instance().IsBaselineRankId(rankIds[0])) ?
        rankIds[0] : filePath;
    auto database = std::dynamic_pointer_cast<DbTraceDataBase, Timeline::VirtualTraceDatabase>(
        Timeline::DataBaseManager::Instance().GetTraceDatabaseByFileId(dbId));
    if (database == nullptr) {
        ServerLog::Error("Failed to get database connection in init open db.");
        return;
    }
    database->AddHelperColumnsAndSetStatus();
    auto &threadPool = FullDbParser::Instance().threadPool;
    std::shared_ptr<std::vector<std::future<void>>> futures = std::make_shared<std::vector<std::future<void>>>();
    FileType type = DataBaseManager::Instance().GetFileTypeByRankId(rankIds[0]);
    if (type != FileType::LEAKS) {
        for (const auto &item: rankIds) {
            database->UpdateStartTime(item);
        }
        database->InitStringsCache();
        BuildProfilingInitTask(futures, dbId, threadPool);
    }
    // EndParseTask中会等待所有future执行完成，然后发送parse/success事件，最后在执行一些需要异步完成的解析任务
    threadPool->AddTask(EndParseTask, TraceIdManager::GetTraceId(), rankIds, filePath, futures, start);

    // Init Memory
    if (type == FileType::MS_PROF && !database->CheckTableDataInvalid(TABLE_OPERATOR_MEMORY)) {
        for (const auto& rankId: rankIds) {
            FullDb::DbMemoryDataBase::ParserEnd(rankId, false, dbId);
            FullDb::DbMemoryDataBase::ParseCallBack(rankId, filePath, false, "");
        }
        ServerLog::Error("There is no Memory Data in this db file");
    } else if (type == FileType::LEAKS && !database->CheckTableDataInvalid(TABLE_LEAKS_DUMP)) {
        for (const auto& rankId: rankIds) {
            MemoryDetail::LeaksMemoryService::ParserEnd(rankId, false);
            MemoryDetail::LeaksMemoryService::ParseCallBack(rankId, false, "There is no Leaks Memory Data in this db file");
        }
        ServerLog::Error("There is no Leaks Memory Data in this db file");
    } else {
        InitMemory(rankIds, filePath);
    }

    // Init Operator
    if (!database->CheckTableDataInvalid(TABLE_COMPUTE_TASK_INFO)) {
        for (const auto& rankId: rankIds) {
            FullDb::DbSummaryDataBase::ParserEnd(rankId, filePath, false, "");
        }
        ServerLog::Error("There is no Summery Data in this db file");
    } else {
        InitSummary(rankIds, filePath);
    }
}
void FullDbParser::BuildProfilingInitTask(std::shared_ptr<std::vector<std::future<void>>> &futures, std::string &dbId,
                                          std::unique_ptr<ThreadPool> &pool)
{
    futures->emplace_back(pool->AddTask([dbId]() {
        std::shared_ptr<DbTraceDataBase> database = GetTraceDatabase(dbId);
        if (!database) {
            return;
        }
        database->InitMetaDataInfo();
    }, TraceIdManager::GetTraceId()));
    futures->emplace_back(pool->AddTask([dbId]() {
        std::shared_ptr<DbTraceDataBase> database = GetTraceDatabase(dbId);
        if (!database) {
            return;
        }
        if (!database->InitStmt()) {
            return;
        }
        // sqlite同一时刻只支持一个数据库连接进行写操作，以下4个线程都是写操作，并发可能导致某个线程写失败
        database->UpdateAllDepth();
    }, TraceIdManager::GetTraceId()));
}

void FullDbParser::EndParseTask(const std::vector<std::string> &rankIds, const std::string &filePath,
    const std::shared_ptr<std::vector<std::future<void>>>& futures,
    std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
    for (const auto &future : *futures) {
        future.wait();
    }
    std::string dbId = (rankIds.size() > 0 && Global::BaselineManager::Instance().IsBaselineRankId(rankIds[0])) ?
        rankIds[0] : filePath;
    for (const std::string& id : rankIds) {
        ParserCallBack(id, filePath, true);
    }

    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Parse completed.",
                    " Cost time(ms): ", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    for (auto rankId: rankIds) {
        Timeline::ParserStatusManager::Instance().SetParserStatus(rankId, Timeline::ParserStatus::FINISH_ALL);
        auto db = DataBaseManager::Instance().GetTraceDatabaseByRankId(rankId);
        if (db == nullptr) {
            ServerLog::Error("Failed to get connection. fileId:Host");
            break;
        }
        db->SetDataBaseVersion();
    }
    // 保证下面任务的执行在发送了parse/success之后
    ParseUnitManager::Instance().ExecuteAll({dbId});
}

void FullDbParser::ParserCallBack(std::string rankId, const std::string &fileId, bool result)
{
    auto &instance = FullDbParser::Instance();
    if (instance.parseEndCallback != nullptr) {
        instance.parseEndCallback(rankId, fileId, true, "");
    }
}

void FullDbParser::InitSummary(const std::vector<std::string> &rankIds, const std::string &path)
{
    for (const std::string& id : rankIds) {
        bool result = false;
        auto summeryDatabase = std::dynamic_pointer_cast<FullDb::DbSummaryDataBase, Summary::VirtualSummaryDataBase>(
            Timeline::DataBaseManager::Instance().CreateSummaryDatabase(id, path));
        if (summeryDatabase != nullptr && summeryDatabase->OpenDb(path, false)) {
            result = true;
        } else {
            ServerLog::Error("Failed to connect or open SummeryDatabase.");
        }
        if (!Global::BaselineManager::Instance().IsBaselineRankId(id)) {
            FullDb::DbSummaryDataBase::ParserEnd(id, path, result, "");
        }
    }
    ServerLog::Info("Init Summary finish");
}

void FullDbParser::InitMemory(const std::vector<std::string> &rankIds, const std::string &path)
{
    FileType type = DataBaseManager::Instance().GetFileTypeByRankId(rankIds[0]);
    if (type == FileType::LEAKS) {
        InitLeaksMemory(rankIds, path);
        return;
    }
    for (const std::string& id : rankIds) {
        bool result = false;
        auto memoryDatabase = std::dynamic_pointer_cast<FullDb::DbMemoryDataBase, Memory::VirtualMemoryDataBase>(
            Timeline::DataBaseManager::Instance().CreateMemoryDataBase(id, path));
        if (memoryDatabase->IsOpen() || memoryDatabase->OpenDb(path, false)) {
            FullDb::DbMemoryDataBase::ParserEnd(id, true, path);
            result = true;
        } else {
            FullDb::DbMemoryDataBase::ParserEnd(id, false, path);
            ServerLog::Error("Failed to connect or open memoryDatabase.");
        }
        if (!Global::BaselineManager::Instance().IsBaselineRankId(id)) {
            FullDb::DbMemoryDataBase::ParseCallBack(id, path, result, "");
        }
    }
    ServerLog::Info("Init Memory finish");
}

bool FullDbParser::Parse(const std::vector<std::string> &fileIds,
                         const std::string &filePath,
                         const std::string &selectedFolder,
                         const std::string &fileId)
{
    return false;
}

void FullDbParser::InitLeaksMemory(const std::vector<std::string> &rankIds, const std::string &path)
{
    for (const std::string& id : rankIds) {
        auto leaksMemoryDatabase = Timeline::DataBaseManager::Instance().GetLeaksMemoryDatabase("");
        if (leaksMemoryDatabase != nullptr && leaksMemoryDatabase->OpenDb(path, false)) {
            if (MemoryDetail::LeaksMemoryService::ParseMemoryLeaksDumpEventsAndPythonTraces(id)) {
                MemoryDetail::LeaksMemoryService::ParserEnd(id, true);
                MemoryDetail::LeaksMemoryService::ParseCallBack(id, true, "");
            } else {
                MemoryDetail::LeaksMemoryService::ParserEnd(id, false);
                MemoryDetail::LeaksMemoryService::ParseCallBack(id, false,
                                                                "An exception occurred while parsing the DB data: "
                                                                "Please check the logs for details.");
                ServerLog::Error("Failed to connect or open leaks memory database.");
            }
        } else {
            MemoryDetail::LeaksMemoryService::ParserEnd(id, false);
            MemoryDetail::LeaksMemoryService::ParseCallBack(id, false,
                                                            "An exception occurred while parsing the DB data: "
                                                            "The database failed to open properly.");
            ServerLog::Error("Failed to connect or open leaks memory database.");
        }
        Timeline::ParserStatusManager::Instance().SetParserStatus(id, Timeline::ParserStatus::FINISH_ALL);
    }
}
}