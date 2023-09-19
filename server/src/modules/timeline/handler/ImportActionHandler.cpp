/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ImportActionHandler.h"

#include "ServerLog.h"
#include "ExecUtil.h"
#include "FileUtil.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "TraceFileParser.h"
#include "ClusterFileParser.h"
#include "MemoryParse.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic;
using namespace Dic::Server;

void ImportActionHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ImportActionRequest &request = dynamic_cast<ImportActionRequest &>(*requestPtr.get());
    std::string token = request.token;
    ServerLog::Info("Import action, token = ", StringUtil::AnonymousString(token));
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    if (request.params.path.empty()) {
        ServerLog::Warn("Import path is empty.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    std::string selectedFolder = request.params.path[0];
    auto traceFiles = FileUtil::FindAllFileByName(request.params.path, selectedFolder, traceViewFile, traceViewReg);
    if (traceFiles.empty()) {
        ServerLog::Error("Failed to find trace file.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    // 按rankId 拆分文件
    std::map<std::string, std::vector<std::string>> rankListMap = FileUtil::SplitToRankList(traceFiles);
    SetParseCallBack(token);
    SetBaseActionOfResponse(rankListMap, response, selectedFolder);
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    for (const auto &rankEntry : rankListMap) {
        TraceFileParser::Instance().Parse(rankEntry.second, rankEntry.first, selectedFolder);
    }
    std::string parseClusterResult = "none";
    if (rankListMap.size() > 1) {
        ClusterFileParser clusterFileParser;
        if (clusterFileParser.ParseClusterFiles(selectedFolder)) {
            parseClusterResult = "ok";
        } else {
            parseClusterResult = "fail";
        }
    }
    // send event
    ParseClusterEndProcess(token, parseClusterResult);
}

void ImportActionHandler::SetBaseActionOfResponse(const std::map<std::string, std::vector<std::string>> &rankListMap,
                                                  ImportActionResponse &response, const std::string &path)
{
    for (const auto &rankEntry : rankListMap) {
        std::string rankId = rankEntry.first;
        Action action;
        if (DataBaseManager::Instance().HasFileId(rankId)) {
            continue;
        }
        action.cardName = rankId;
        action.rankId = rankId;
        action.result = true;
        if (HasMemoryFile(path)) {
            action.hasMemory = true;
        }
        response.body.result.emplace_back(action);
    }
}

bool ImportActionHandler::HasMemoryFile(const std::string& path)
{
    auto operatorFiles = FileUtil::FindFileByName(path, memoryOperatorFile, memoryOperatorReg);
    auto recordFiles = FileUtil::FindFileByName(path, memoryRecordFile, memoryRecordReg);
    if (!operatorFiles.empty() or !recordFiles.empty()) {
        return true;
    }
    return false;
}

void ImportActionHandler::ParseClusterEndProcess(const std::string token, std::string result)
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

void ImportActionHandler::ParseEndCallBack(const std::string token, const std::string fileId,
                                           bool result)
{
    ServerLog::Info("Parse end, token = ", StringUtil::AnonymousString(token), " fileId:", fileId,
                    ", result:", result);
    WsSession *session = WsSessionManager::Instance().GetSession(token);
    if (session == nullptr) {
        ServerLog::Warn("Failed to get session, token = ", StringUtil::AnonymousString(token));
        return;
    }
    auto event = std::make_unique<ParseSuccessEvent>();
    event->moduleName = ModuleType::TIMELINE;
    event->token = token;
    event->result = result;
    event->body.unit.type = "card";
    event->body.unit.metadata.cardId = fileId;
    uint64_t min = UINT64_MAX;
    uint64_t max = 0;
    DataBaseManager::Instance().GetTraceDatabase(fileId)->QueryExtremumTimestamp(min, max);
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

void ImportActionHandler::SetParseCallBack(const std::string &token)
{
    static bool flag = false;
    if (!flag) {
        flag = true;
        std::function<void(const std::string, bool)> func =
                std::bind(ParseEndCallBack, token, std::placeholders::_1, std::placeholders::_2);
        TraceFileParser::Instance().SetParseEndCallBack(func);
    }
}

void ImportActionHandler::SearchMetaData(const std::string &fileId,
                                         std::vector<std::unique_ptr<UnitTrack>> &metaData)
{
    DataBaseManager::Instance().GetTraceDatabase(fileId)->QueryUnitsMetadata(fileId, metaData);
}

} // Timeline
} // Module
} // Dic