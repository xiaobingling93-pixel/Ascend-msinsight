/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "ParserStatusManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "DbTraceDataBase.h"
#include "DbMemoryDataBase.h"
#include "DbSummaryDataBase.h"
#include "ClusterParseThreadPoolExecutor.h"
#include "CommonCacheManager.h"
#include "BaselineManager.h"
#include "CollectionTimeService.h"
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

bool FullDbParser::Parse(const std::vector<std::string> &fileIds, const std::string &filePath)
{
    ServerLog::Info("start db parse.");
    for (auto id:fileIds) {
        Timeline::ParserStatusManager::Instance().SetParserStatus(id, Timeline::ParserStatus::INIT);
    }
    auto &instance = FullDbParser::Instance();
    instance.threadPool->AddTask(InitOpenDb, filePath, fileIds);
    return true;
}

void FullDbParser::Reset()
{
    ServerLog::Info("Reset. wait task completed.");
    Timeline::ParserStatusManager::Instance().SetAllTerminateStatus();
    Timeline::ParserStatusManager::Instance().SetClusterParseStatus(Timeline::ParserStatus::TERMINATE);
    ServerLog::Info("Task completed.");
    auto connList = Timeline::DataBaseManager::Instance().GetAllTraceDatabase();
    for (auto &conn : connList) {
        conn->Stop();
    }
    Timeline::DataBaseManager::Instance().Clear();
    Timeline::TraceTime::Instance().Reset();
    FileParser::Reset();
    Timeline::ParserStatusManager::Instance().ClearAllParserStatus();
    FullDb::DbMemoryDataBase::Reset();
    FullDb::DbSummaryDataBase::Reset();
    FullDb::DbTraceDataBase::Reset();
    CommonCacheManager::Instance().Clear();
    ServerLog::Info("End Reset trace Parser");
    threadPool->Reset();
    CollectionTimeService::Instance().Reset();
}

// 此方法为私有方法，调用前需保证不会出现空指针的情况
std::shared_ptr<DbTraceDataBase> FullDbParser::GetTraceDatabase(const std::string &filePath)
{
    auto db = Timeline::DataBaseManager::Instance().GetTraceDatabase(filePath);
    return std::dynamic_pointer_cast<DbTraceDataBase, Timeline::VirtualTraceDatabase>(db);
}

void FullDbParser::InitOpenDb(const std::string &filePath, const std::vector<std::string> &rankIds)
{
    auto start = std::chrono::high_resolution_clock::now();
    std::string dbId = (rankIds.size() > 0 && Global::BaselineManager::Instance().IsBaselineId(rankIds[0])) ?
        rankIds[0] : filePath;
    auto db = Timeline::DataBaseManager::Instance().GetTraceDatabase(dbId);
    if (db == nullptr) {
        ServerLog::Error("Failed to get connection.");
        return;
    }
    auto database = std::dynamic_pointer_cast<DbTraceDataBase, Timeline::VirtualTraceDatabase>(db);
    if (database == nullptr) {
        ServerLog::Error("Failed to convert virtual trace database to db trace dataBase in init open db.");
        return;
    }
    auto &threadPool = FullDbParser::Instance().threadPool;
    std::shared_ptr<std::vector<std::future<void>>> futures = std::make_shared<std::vector<std::future<void>>>();
    futures->emplace_back(threadPool->AddTask([dbId]() { GetTraceDatabase(dbId)->InitStringsCache(); }));
    futures->emplace_back(threadPool->AddTask([dbId]() { GetTraceDatabase(dbId)->UpdateAllDepth(); }));
    futures->emplace_back(threadPool->AddTask([dbId]() { GetTraceDatabase(dbId)->UpdateWaitTime(); }));
    futures->emplace_back(threadPool->AddTask([dbId]() { GetTraceDatabase(dbId)->GenerateOverlapAnalysis(); }));

    threadPool->AddTask(EndParseTask, rankIds, filePath, futures, start);

    database->UpdateStartTime(rankIds[0]);

    FileType type = DataBaseManager::Instance().GetFileTypeByRankId(rankIds[0]);
    if (type == FileType::MS_PROF && !database->CheckTableDataInvalid(TABLE_OPERATOR_MEMORY)) {
        for (const auto& rankId: rankIds) {
            FullDb::DbMemoryDataBase::ParserEnd(rankId, false);
            FullDb::DbMemoryDataBase::ParseCallBack(rankId, false, "");
        }
        ServerLog::Error("There is no Memory Data in this db file");
    } else {
        InitMemory(rankIds, filePath);
    }
    if (!database->CheckTableDataInvalid(TABLE_COMPUTE_TASK_INFO)) {
        for (const auto& rankId: rankIds) {
            FullDb::DbSummaryDataBase::ParserEnd(rankId, false, "");
        }
        ServerLog::Error("There is no Summery Data in this db file");
    } else {
        InitSummary(rankIds, filePath);
    }
}

void FullDbParser::EndParseTask(const std::vector<std::string> &rankIds, const std::string &filePath,
    const std::shared_ptr<std::vector<std::future<void>>>& futures,
    std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
    for (const auto &future : *futures) {
        future.wait();
    }
    std::string dbId = (rankIds.size() > 0 && Global::BaselineManager::Instance().IsBaselineId(rankIds[0])) ?
        rankIds[0] : filePath;
    bool isNotSendMessage = Global::BaselineManager::Instance().IsBaselineId(rankIds[0])
            && DataBaseManager::Instance().IsContainDatabasePath(filePath);
    FullDbParser::Instance().threadPool->AddTask([dbId]() { GetTraceDatabase(dbId)->InitFlowCache(); });
    for (const std::string& id : rankIds) {
        ParserCallBack(id, true);
    }

    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Parse completed.",
                    " Cost time(ms): ", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    if (!isNotSendMessage) {
        SendHostEvent(dbId);
    }
    for (auto rankId: rankIds) {
        Timeline::ParserStatusManager::Instance().SetParserStatus(rankId, Timeline::ParserStatus::FINISH_ALL);
    }
}

void FullDbParser::SendHostEvent(const std::string &fileId)
{
    WsSession *session = WsSessionManager::Instance().GetSession();
    if (session == nullptr) {
        ServerLog::Warn("Failed to get session");
        return;
    }
    auto event = std::make_unique<ParseSuccessEvent>();
    event->moduleName = ModuleType::TIMELINE;
    event->result = true;
    event->body.unit.type = "card";
    event->body.isFullDb = true;
    auto db = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (db == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:Host");
        return;
    }
    auto database = std::dynamic_pointer_cast<FullDb::DbTraceDataBase, Timeline::VirtualTraceDatabase>(db);
    if (database == nullptr) {
        ServerLog::Error("Failed to convert virtual trace database to db trace dataBase in full db send host event.");
        return;
    }
    event->body.unit.metadata.cardId = database->QueryHostInfo()+"Host";
    event->body.maxTimeStamp = TraceTime::Instance().GetDuration();
    database->QueryHostMetadata(event->body.unit.children);
    session->OnEvent(std::move(event));
}

void FullDbParser::ParserCallBack(std::string fileId, bool result)
{
    auto &instance = FullDbParser::Instance();
    if (instance.paserEndCallback != nullptr) {
        instance.paserEndCallback(fileId, true, "");
    }
}

void FullDbParser::InitSummary(const std::vector<std::string> &rankIds, const std::string &path)
{
    for (const std::string& id : rankIds) {
        bool result = false;
        auto summeryDatabase = dynamic_cast<FullDb::DbSummaryDataBase *>(
                Timeline::DataBaseManager::Instance().GetSummaryDatabase(id));
        if (summeryDatabase != nullptr && summeryDatabase->OpenDb(path, false)) {
            result = true;
        } else {
            ServerLog::Error("Failed to connect or open SummeryDatabase.");
        }
        if (!Global::BaselineManager::Instance().IsBaselineId(id)) {
            FullDb::DbSummaryDataBase::ParserEnd(id, result, "");
        }
    }
    ServerLog::Info("Init Summary finish");
}

void FullDbParser::InitMemory(const std::vector<std::string> &rankIds, const std::string &path)
{
    for (const std::string& id : rankIds) {
        bool result = false;
        auto memoryDatabase = dynamic_cast<FullDb::DbMemoryDataBase *>(
                Timeline::DataBaseManager::Instance().GetMemoryDatabase(id));
        if (memoryDatabase != nullptr && memoryDatabase->OpenDb(path, false)) {
            FullDb::DbMemoryDataBase::ParserEnd(id, true);
            result = true;
        } else {
            FullDb::DbMemoryDataBase::ParserEnd(id, false);
            ServerLog::Error("Failed to connect or open memoryDatabase.");
        }
        if (!Global::BaselineManager::Instance().IsBaselineId(id)) {
            FullDb::DbMemoryDataBase::ParseCallBack(id, result, "");
        }
    }
    ServerLog::Info("Init Memory finish");
}

bool FullDbParser::Parse(const std::vector<std::string> &fileIds, const std::string &filePath,
                         const std::string &selectedFolder)
{
    return false;
}
}