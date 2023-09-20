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
    ImportActionResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    if (request.params.path.empty()) {
        ServerLog::Warn("Import path is empty.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    std::string selectedFolder = request.params.path[0];
    auto files = GetTraceFiles(request.params.path, response.body);
    if (files.empty()) {
        ServerLog::Warn("Import files is empty.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    // 按rankId 拆分文件
    std::map<std::string, std::vector<std::string>> rankListMap = FileUtil::SplitToRankList(files);
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

std::vector<std::string> ImportActionHandler::FindAllTraceFile(const std::vector<std::string> &pathList)
{
    std::vector<std::string> traceFiles;
    for (const auto &path : pathList) {
        if (path == "browser") {
            return FindTraceFile(ExecUtil::SelectFolder());
        }
        auto files = FindTraceFile(path);
        if (files.empty()) {
            ServerLog::Warn("Can't find trace file in path:", path);
            continue;
        }
        traceFiles.insert(traceFiles.end(), files.begin(), files.end());
    }
    return traceFiles;
}

std::vector<std::string> ImportActionHandler::FindTraceFile(const std::string &path)
{
    std::vector<std::string> traceFiles;
    if (!FileUtil::IsFolder(path)) {
        traceFiles.emplace_back(path);
        return traceFiles;
    }
    std::function<void(const std::string&, int)> find = [&find, this, &traceFiles](const std::string &path, int depth) {
        if (depth > 5) {
            return;
        }
        auto folders = FileUtil::FindFolders(path);
        if (std::find(folders.begin(), folders.end(), "ASCEND_PROFILER_OUTPUT") != folders.end()) {
            FindAscendFolder(path, traceFiles);
            return;
        }
        for (const auto &folder : folders) {
            std::string tmpPath = FileUtil::SplicePath(path, folder);
            if (FileUtil::IsFolder(tmpPath)) {
                find(tmpPath, depth + 1);
            } else if (IsJsonValid(folder)) {
                traceFiles.push_back(tmpPath);
            }
        }
    };
    find(path, 0);
    return traceFiles;
}

bool ImportActionHandler::IsJsonValid(const std::string &fileName)
{
    static std::string reg = R"((trace_view|msprof.*)\.json$)";
    auto result = RegexUtil::RegexMatch(fileName, reg);
    return result.has_value();
}

void ImportActionHandler::FindAscendFolder(const std::string &path, std::vector<std::string> &traceFiles)
{
    std::string traceFilePath = FileUtil::SplicePath(path, "ASCEND_PROFILER_OUTPUT");
    traceFilePath = FileUtil::SplicePath(traceFilePath, "trace_view.json");
    ServerLog::Info("FindAscendFolder. ", traceFilePath);
    if (FileUtil::CheckDirectoryExist(traceFilePath)) {
        traceFiles.emplace_back(traceFilePath);
        return;
    }
    std::function<void(const std::string&, int)> find = [&find, this, &traceFiles](const std::string &path, int depth) {
        if (depth > 5) {
            return;
        }
        auto folders = FileUtil::FindFolders(path);
        for (const auto &folder : folders) {
            std::string tmpPath = FileUtil::SplicePath(path, folder);
            if (FileUtil::IsFolder(tmpPath)) {
                find(tmpPath, depth + 1);
            } else if (IsJsonValid(folder)) {
                traceFiles.push_back(tmpPath);
            }
        }
    };
    auto folders = FileUtil::FindFolders(path);
    static std::string reg = R"(PROF_.*)";
    for (const auto &folder : folders) {
        if (!RegexUtil::RegexMatch(folder, reg).has_value()) {
            continue;
        }
        std::string tmpPath = FileUtil::SplicePath(path, folder);
        if (FileUtil::IsFolder(tmpPath)) {
            find(tmpPath, 0);
        }
        break;
    }
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

std::string ImportActionHandler::GetFileId(const std::string &filePath)
{
    std::string fileId = TraceFileParser::Instance().GetFileId(filePath);
    int i = 1;
    std::string result = fileId;
    while (DataBaseManager::Instance().HasFileId(result)) {
        result = fileId + "_" + std::to_string(++i);
    }
    auto database = DataBaseManager::Instance().GetTraceDatabase(result);
    return database == nullptr ? "" : result;
}

bool ImportActionHandler::CheckIsCluster(const std::string &filePath)
{
    auto folders = FileUtil::FindFolders(filePath);
    for (const auto &folder : folders) {
        if (folder == "cluster_analysis_output") {
            return true;
        }
    }
    return false;
}

std::vector<std::pair<std::string, std::string>> ImportActionHandler::GetTraceFiles(
    const std::vector<std::string> &pathList, ImportActionResBody &body)
{
    auto traceFiles = FindAllTraceFile(pathList);
    if (traceFiles.empty()) {
        ServerLog::Error("Failed to find trace file.");
        return {};
    }
    if (pathList.size() == 1) {
        bool isCluster = CheckIsCluster(pathList[0]);
        bool reset = isCluster || curIsCluster;
        ServerLog::Info("new Cluster:", isCluster, ", old Cluster:", curIsCluster, ", reset:", reset);
        curIsCluster = isCluster;
        if (reset) {
            TraceFileParser::Instance().Reset();
            body.reset = reset;
        }
        body.isCluster = isCluster;
    } else {
        body.isCluster = false;
    }
    std::vector<std::pair<std::string, std::string>> files;
    for (const auto &file : traceFiles) {
        std::string fileId = GetFileId(file);
        if (fileId.empty()) {
            ServerLog::Error("File id is empty. file:", file);
            continue;
        }
        files.emplace_back(file, fileId);
    }
    return files;
}
} // Timeline
} // Module
} // Dic