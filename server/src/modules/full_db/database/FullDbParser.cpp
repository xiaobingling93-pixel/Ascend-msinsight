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
#include "DbClusterDataBase.h"

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

bool FullDbParser::Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
                         const std::string &token)
{
    ServerLog::Info("start db parse.");
    Timeline::ParserStatusManager::Instance().SetParserStatus(fileId, Timeline::ParserStatus::INIT);
    auto &instance = FullDbParser::Instance();
    instance.threadPool->AddTask(InitOpenDb, filePaths[0], fileId, token);
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

void FullDbParser::InitOpenDb(const std::string &filePath, const std::string &rankId,
                              const std::string& token)
{
    ServerLog::Info(filePath);

    auto database = std::dynamic_pointer_cast<DbTraceDataBase, Timeline::VirtualTraceDatabase>(
        Timeline::DataBaseManager::Instance().GetTraceDatabase("FullDb"));
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection.");
        return;
    }
    database->UpdateAllTaskDepth();
    database->UpdateAllApiDepth();
    database->InitStringsCache();
    database->UpdateStartTime();

    std::vector<std::string> rankIds = database->QueryRankId();

    if (!database->CheckTableDataInvalid(TABLE_MEMORY_RECORD + "," + TABLE_OPERATOR_MEMORY)) {
        FullDb::DbMemoryDataBase::ParserEnd(rankId, false);
        FullDb::DbMemoryDataBase::ParseCallBack(token, rankId, false, "");
        ServerLog::Error("There is no Memory Data in this db file:" + filePath);
    } else {
        InitMemory(rankIds, filePath, token);
    }

    if (!database->CheckTableDataInvalid(TABLE_COMPUTE_TASK_INFO)) {
        FullDb::DbSummaryDataBase::ParserEnd(token, rankId, false, "");
        ServerLog::Error("There is no Summery Data in this db file:" + filePath);
    } else {
        InitSummery(rankIds, filePath, token);
    }

    for (const std::string& id : rankIds) {
        ParserCallBack(id, true);
        Timeline::ParserStatusManager::Instance().SetParserStatus(id, Timeline::ParserStatus::FINISH_ALL);
    }
    SendHostEvent(token);
}

void FullDbParser::SendHostEvent(const std::string &token)
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
    auto database = std::dynamic_pointer_cast<FullDb::DbTraceDataBase, Timeline::VirtualTraceDatabase>(
        DataBaseManager::Instance().GetTraceDatabase("Host"));
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:Host");
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
    auto summeryDatabase = dynamic_cast<FullDb::DbSummaryDataBase *>(
            Timeline::DataBaseManager::Instance().GetSummaryDatabase("FullDb"));
    if (summeryDatabase == nullptr) {
        ServerLog::Error("Failed to get summery connection.");
    }
    if (!summeryDatabase->OpenDb(path, false)) {
        ServerLog::Error("Failed to open SummaryDataBase. rankId:", "FullDb");
    }
    for (const std::string& id : rankIds) {
        FullDb::DbSummaryDataBase::ParserEnd(token, id, true, "");
    }
    ServerLog::Info("Init Summary finish");
}

void FullDbParser::InitMemory(std::vector<std::string> rankIds, std::string path, std::string token)
{
    auto memoryDatabase = dynamic_cast<FullDb::DbMemoryDataBase *>(
            Timeline::DataBaseManager::Instance().GetMemoryDatabase("FullDb"));
    if (memoryDatabase == nullptr) {
        ServerLog::Error("Failed to get memory connection.");
    }
    if (!memoryDatabase->OpenDb(path, false)) {
        ServerLog::Error("Failed to open memoryDatabase. rankId:", "FullDb");
    }
    memoryDatabase->SetInferenceType(false);
    for (const std::string& id : rankIds) {
        FullDb::DbMemoryDataBase::ParserEnd(id, true);
        FullDb::DbMemoryDataBase::ParseCallBack(token, id, true, "");
    }
    ServerLog::Info("Init Memory finish");
}

bool FullDbParser::InitCluster(std::string path, std::string token)
{
    auto clusterDatabase = dynamic_cast<DbClusterDataBase*>(DataBaseManager::Instance().GetReadClusterDatabase());
    if (clusterDatabase == nullptr) {
        ServerLog::Error("Failed to get Cluster connection.");
        return false;
    }
    if (!clusterDatabase->OpenDb(path, false)) {
        ServerLog::Error("Failed to open Cluster. rankId:", "FullDb");
        return false;
    }
    clusterDatabase->UpdateClusterParseStatus(FINISH_STATUS);
    ServerLog::Info("ParseClusterFiles is success");
    return true;
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