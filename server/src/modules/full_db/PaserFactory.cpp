/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "ParserFactory.h"
#include "ParserBin.h"
#include "ParserJson.h"
#include "ParserIpynb.h"
#include "ParserDb.h"
#include "ServerLog.h"
#include "ExecUtil.h"
#include "FileUtil.h"
#include "RegexUtil.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "ParserStatusManager.h"
#include "ClusterParseThreadPoolExecutor.h"
#include "TraceTime.h"
#include "TraceFileParser.h"
#include "ClusterFileParser.h"
#include "MemoryParse.h"
#include "OperatorProtocolEvent.h"
#include "FullDbParser.h"
#include "DbMemoryDataBase.h"

namespace Dic {
namespace Module {
using namespace Dic;
using namespace Dic::Server;
using namespace Dic::Module::Timeline;

std::shared_ptr<ParserAlloc> ParserFactory::ParserImport(ParserType allocType)
{
    std::shared_ptr<ParserAlloc> alloc;
    switch (allocType) {
        case ParserType::DB:
            alloc = std::make_shared<ParserDb>();
            break;
        case ParserType::BIN:
            alloc = std::make_shared<ParserBin>();
            break;
        case ParserType::JSON:
            alloc = std::make_shared<ParserJson>();
            break;
        case ParserType::IPYNB:
            alloc = std::make_shared<ParserIpynb>();
            break;
        default:
            alloc = std::make_shared<ParserJson>();
            break;
    }
    return alloc;
}

void ParserAlloc::SetBaseActionOfResponse(ImportActionResponse &response,
    std::pair<std::string, std::vector<std::string>> rankEntry)
{
    Action action;
    std::string rankId = rankEntry.first;
    action.cardName = rankId;
    action.rankId = rankId;
    action.result = true;
    if (std::empty(rankEntry.second)) {
        ServerLog::Warn("CardPath is empty, rankId is: ", rankId);
        return;
    }
    // 将文件所在路径的三级目录名称作为rank的tooltip信息
    action.cardPath = "Directory: " + FileUtil::GetRankIdFromPath(rankEntry.second[0]);
    response.body.result.emplace_back(action);
}

void ParserAlloc::ParseClusterEndProcess(const std::string token, std::string result)
{
    ServerLog::Info("Parse Cluster File end, send event");
    WsSession *session = WsSessionManager::Instance().GetSession(token);
    if (session == nullptr) {
        ServerLog::Warn("Failed to get session token ");
        return;
    }
    auto event = std::make_unique<ParseClusterCompletedEvent>();
    event->moduleName = ModuleType::TIMELINE;
    event->token = token;
    event->result = true;
    event->body.parseResult = std::move(result);
    session->OnEvent(std::move(event));
}

void ParserAlloc::ParseEndCallBack(const std::string &token, const std::string &fileId, bool result,
    const std::string &message)
{
    ServerLog::Info("Parse end, token = ", StringUtil::AnonymousString(token), " fileId:", fileId, ", result:", result);
    if (result) {
        SendParseSuccessEvent(token, fileId);
    } else {
        SendParseFailEvent(token, fileId, message);
    }
}

void ParserAlloc::SendParseSuccessEvent(const std::string &token, const std::string &fileId)
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
    event->body.unit.metadata.cardId = fileId;
    uint64_t min = UINT64_MAX;
    uint64_t max = 0;
    auto database = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", fileId);
        return;
    }
    database->QueryExtremumTimestamp(min, max);
    if (min == max && max == 0) {
        event->body.startTimeUpdated = false;
    } else {
        event->body.startTimeUpdated = true;
        TraceTime::Instance().UpdateTime(min, max);
    }
    event->body.maxTimeStamp = TraceTime::Instance().GetDuration();
    SearchMetaData(fileId, event->body.unit.children);
    session->OnEvent(std::move(event));
}

void ParserAlloc::SendParseFailEvent(const std::string &token, const std::string &fileId, const std::string &message)
{
    WsSession *session = WsSessionManager::Instance().GetSession(token);
    if (session == nullptr) {
        ServerLog::Warn("Failed to get session, token = ", StringUtil::AnonymousString(token));
        return;
    }
    auto event = std::make_unique<ParseFailEvent>();
    event->moduleName = ModuleType::TIMELINE;
    event->token = token;
    event->result = true;
    event->body.rankId = fileId;
    event->body.error = message;
    session->OnEvent(std::move(event));
}

void ParserAlloc::SearchMetaData(const std::string &fileId, std::vector<std::unique_ptr<UnitTrack>> &metaData)
{
    auto database = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", fileId);
        return;
    }
    database->QueryUnitsMetadata(fileId, metaData);
}

std::string ParserAlloc::GetFileId(const std::string &filePath)
{
    std::string fileId = FileUtil::GetRankIdFromFile(filePath);
    int i = 1;
    std::string result = fileId;
    std::string parentDir = FileUtil::GetParentPath(filePath);
    std::string name = FileUtil::GetFileName(filePath);
    while (DataBaseManager::Instance().HasFileId(DatabaseType::TRACE, result)) {
        auto database = DataBaseManager::Instance().GetTraceDatabase(result);
        if (database == nullptr) {
            continue;
        }
        std::string dir = FileUtil::GetParentPath(database->GetDbPath());
        if (RegexUtil::RegexMatch(name, R"(^msprof_slice_[0-9_]+\.json$)") && parentDir == dir) {
            return result;
        }
        result = fileId + "_" + std::to_string(++i);
    }
    std::string dbPath = FileUtil::GetDbPath(filePath, result);
    if (!DataBaseManager::Instance().CreatConnectionPool(result, dbPath)) {
        ServerLog::Error("Failed to create connection pool. fileId:", result, ". path:", dbPath);
        return "";
    }
    return result;
}

bool ParserAlloc::CheckIsCluster(const std::string &filePath)
{
    std::vector<std::string> folders;
    std::vector<std::string> files;
    if (filePath.find("cluster_analysis_output") != std::string::npos) {
        ServerLog::Info("this folder is cluster_analysis_output,CheckIsCluster is true");
        return true;
    }
    if (!FileUtil::FindFolders(filePath, folders, files)) {
        ServerLog::Info("FindFolders is empty,CheckIsCluster is false");
        return false;
    }
    return std::any_of(folders.begin(), folders.end(),
        [](std::string &folder) { return folder == "cluster_analysis_output"; });
}

std::string ParserAlloc::GetDbPath(const std::string &filePath, const int index)
{
    std::string path(filePath);
    std::string suffix = DB_FILE_SUFFIX;
    if (index != 1) {
        suffix = "_" + std::to_string(index) + suffix;
    }
    auto pos = path.find_last_of('.');
    if (pos != std::string::npos) {
        path.replace(pos, path.size() - pos, suffix);
    } else {
        path.append(suffix);
    }
    return path;
}

bool ParserAlloc::CheckIfClusterAndReset(const std::string &path, int filesSize, ImportActionResBody &body, bool isDb)
{
    bool isCluster = (filesSize > 1 && std::strcmp(curScene.c_str(), "train") == 0) || CheckIsCluster(path);
    bool reset = isCluster || DataBaseManager::Instance().curIsCluster || isDb || DataBaseManager::Instance().curIsDb;
    ServerLog::Info("new Cluster:", isCluster, ", old Cluster:", DataBaseManager::Instance().curIsCluster,
                    ", reset:", reset);
    if (reset) {
        if (isDb) {
            FullDb::FullDbParser::Instance().Reset();
        } else {
            TraceFileParser::Instance().Reset();
            Summary::KernelParse::Instance().Reset();
            Memory::MemoryParse::Instance().Reset();
        }
        body.reset = true;
    }
    DataBaseManager::Instance().curIsCluster = isCluster;
    DataBaseManager::Instance().curIsDb = isDb;
    body.isCluster = isCluster;
}
} // Module
} // Dic
