/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "FullDbParser.h"

#include <utility>
#include "ServerLog.h"
#include "ParserStatusManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "DbTraceDataBase.h"
#include "DbMemoryDataBase.h"
#include "DbSummaryDataBase.h"
#include "FileUtil.h"
#include "ClusterParseThreadPoolExecutor.h"

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

bool FullDbParser::Parse(const std::vector<std::string> &fileIds, const std::string &filePath,
                         const std::string &token)
{
    ServerLog::Info("start db parse.");
    for (auto id:fileIds) {
        Timeline::ParserStatusManager::Instance().SetParserStatus(id, Timeline::ParserStatus::INIT);
    }
    auto &instance = FullDbParser::Instance();
    instance.threadPool->AddTask(InitOpenDb, filePath, fileIds, token);
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
    ServerLog::Info("End Reset trace Parser");
    threadPool->Reset();
}

// 此方法为私有方法，调用前需保证不会出现空指针的情况
std::shared_ptr<DbTraceDataBase> FullDbParser::GetTraceDatabase(const std::string &filePath)
{
    auto db = Timeline::DataBaseManager::Instance().GetTraceDatabase(filePath);
    return std::dynamic_pointer_cast<DbTraceDataBase, Timeline::VirtualTraceDatabase>(db);
}

void FullDbParser::InitOpenDb(const std::string &filePath, const std::vector<std::string> &rankIds,
                              const std::string& token)
{
    ServerLog::Info(filePath);
    auto start = std::chrono::high_resolution_clock::now();
    auto db = Timeline::DataBaseManager::Instance().GetTraceDatabase(filePath);
    if (db == nullptr) {
        ServerLog::Error("Failed to get connection.");
        return;
    }
    auto database = std::dynamic_pointer_cast<DbTraceDataBase, Timeline::VirtualTraceDatabase>(db);
    if (database == nullptr) {
        ServerLog::Error("Failed to convert VirtualTraceDatabase to DbTraceDataBase in InitOpenDb.");
        return;
    }
    auto &threadPool = FullDbParser::Instance().threadPool;
    std::shared_ptr<std::vector<std::future<void>>> futures = std::make_shared<std::vector<std::future<void>>>();
    futures->emplace_back(threadPool->AddTask([filePath]() { GetTraceDatabase(filePath)->InitStringsCache(); }));
    futures->emplace_back(threadPool->AddTask([filePath]() { GetTraceDatabase(filePath)->UpdateAllDepth(); }));
    futures->emplace_back(threadPool->AddTask([filePath]() { GetTraceDatabase(filePath)->UpdateWaitTime(); }));
    futures->emplace_back(threadPool->AddTask([filePath]() { GetTraceDatabase(filePath)->GenerateOverlapAnalysis(); }));

    threadPool->AddTask(EndParseTask, rankIds, filePath, futures, token, start);

    database->UpdateStartTime();

    FileType type = DataBaseManager::Instance().GetFileType();
    if (type == FileType::MS_PROF && !database->CheckTableDataInvalid(TABLE_OPERATOR_MEMORY)) {
        for (const auto& rankId: rankIds) {
            FullDb::DbMemoryDataBase::ParserEnd(rankId, false);
            FullDb::DbMemoryDataBase::ParseCallBack(token, rankId, false, "");
        }
        ServerLog::Error("There is no Memory Data in this db file:" + filePath);
    } else {
        InitMemory(rankIds, filePath, token);
    }

    if (!database->CheckTableDataInvalid(TABLE_COMPUTE_TASK_INFO)) {
        for (const auto& rankId: rankIds) {
            FullDb::DbSummaryDataBase::ParserEnd(token, rankId, false, "");
        }
        ServerLog::Error("There is no Summery Data in this db file:" + filePath);
    } else {
        InitSummery(rankIds, filePath, token);
    }
}

void FullDbParser::EndParseTask(const std::vector<std::string> &rankIds, const std::string &filePath,
    const std::shared_ptr<std::vector<std::future<void>>>& futures, const std::string& token,
    std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
    for (const auto &future : *futures) {
        future.wait();
    }

    for (const std::string& id : rankIds) {
        ParserCallBack(id, true);
    }
    for (auto rankId: rankIds) {
        Timeline::ParserStatusManager::Instance().SetParserStatus(rankId, Timeline::ParserStatus::FINISH_ALL);
    }

    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Parse completed. path:", filePath,
                    " Cost time(ms): ", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    ParserCallBack(filePath, true);
    SendHostEvent(token, filePath);
}

void FullDbParser::SendHostEvent(const std::string &token, const std::string &fileId)
{
    WsSession *session = WsSessionManager::Instance().GetSession(token);
    if (session == nullptr) {
        ServerLog::Warn("Failed to get session, token = ", StringUtil::AnonymousString(token));
        return;
    }
    auto event = std::make_unique<ParseSuccessEvent>();
    event->moduleName = ModuleType::TIMELINE;
    event->token = token;
    event->result = true;
    event->body.unit.type = "card";
    event->body.unit.metadata.cardId = "Host";
    event->body.isFullDb = true;
    auto db = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (db == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:Host");
        return;
    }
    auto database = std::dynamic_pointer_cast<FullDb::DbTraceDataBase, Timeline::VirtualTraceDatabase>(db);
    if (database == nullptr) {
        ServerLog::Error("Failed to convert VirtualTraceDatabase to DbTraceDataBase in full db SendHostEvent.");
        return;
    }
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

void FullDbParser::InitSummery(std::vector<std::string> rankIds, std::string path, std::string token)
{
    for (const std::string& id : rankIds) {
        auto summeryDatabase = dynamic_cast<FullDb::DbSummaryDataBase *>(
                Timeline::DataBaseManager::Instance().GetSummaryDatabase(id));
        if (summeryDatabase != nullptr && summeryDatabase->OpenDb(path, false)) {
            FullDb::DbSummaryDataBase::ParserEnd(token, id, true, "");
        } else {
            FullDb::DbSummaryDataBase::ParserEnd(token, id, false, "");
            ServerLog::Error("Failed to connect or open SummeryDatabase. rankId:", id);
        }
    }
    ServerLog::Info("Init Summary finish");
}

void FullDbParser::InitMemory(std::vector<std::string> rankIds, std::string path, std::string token)
{
    for (const std::string& id : rankIds) {
        auto memoryDatabase = dynamic_cast<FullDb::DbMemoryDataBase *>(
                Timeline::DataBaseManager::Instance().GetMemoryDatabase(id));
        if (memoryDatabase != nullptr && memoryDatabase->OpenDb(path, false)) {
            FullDb::DbMemoryDataBase::ParserEnd(id, true);
            FullDb::DbMemoryDataBase::ParseCallBack(token, id, true, "");
        } else {
            FullDb::DbMemoryDataBase::ParserEnd(id, false);
            FullDb::DbMemoryDataBase::ParseCallBack(token, id, false, "");
            ServerLog::Error("Failed to connect or open memoryDatabase. rankId:", id);
        }
    }
    ServerLog::Info("Init Memory finish");
}

bool FullDbParser::FindDevicePaths(const std::string &selectedFolder,
                                   std::map<std::string, std::string> &devicePaths)
{
    static const std::string DeviceReg = R"(device_\d{1,4})";
    auto folders = FileUtil::FindFilesAndFoldersByRegex(selectedFolder, std::regex(DeviceReg), true);
    for (const auto &folder: folders) {
        auto folderName = FileUtil::GetFileName(folder);
        auto index = folderName.find('_');
        if (index == std::string::npos) {
            return false;
        }
        auto deviceId = folderName.substr(index + 1);
        devicePaths[deviceId] = FileUtil::GetParentPath(folder);
    }
}
}