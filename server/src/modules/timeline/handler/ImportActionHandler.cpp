/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ImportActionHandler.h"
#include "ServerLog.h"
#include "ExecUtil.h"
#include "FileDef.h"
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
#include "KernelParse.h"
#include "SourceFileParser.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic;
using namespace Dic::Server;

bool ImportActionHandler::curIsCluster = false;

void ImportActionHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ImportActionRequest &request = dynamic_cast<ImportActionRequest &>(*requestPtr.get());
    std::string token = request.token;
    ServerLog::Info("Import action request handler start");
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

    // 算子开发场景
    if (Source::SourceFileParser::Instance().CheckOperatorBinary(selectedFolder)) {
        ServerLog::Info("Import files is binary.Start parse source binary file.");
        HandleCompute(response, selectedFolder);
        session.OnResponse(std::move(responsePtr));
        return;
    }

    // 模型调优场景
    auto files = GetTraceFiles(request.params.path, response.body);
    if (files.empty()) {
        ServerLog::Warn("Import files is empty.");
        SetResponseResult(response, true);
        session.OnResponse(std::move(responsePtr));
    } else {
        // 按rankId 拆分文件
        std::map<std::string, std::vector<std::string>> rankListMap = FileUtil::SplitToRankList(files);
        SetBaseActionOfResponse(rankListMap, response);
        SetParseCallBack(token);
        SetResponseResult(response, true);
        // add response to response queue in session
        session.OnResponse(std::move(responsePtr));
        for (const auto &rankEntry : rankListMap) {
            TraceFileParser::Instance().Parse(rankEntry.second, rankEntry.first, selectedFolder);
        }
    }

    if (!Summary::KernelParse::Instance().Parse(request.params.path, token)) {
        ServerLog::Warn("Failed to parse kernel files.");
    }

    if (!Memory::MemoryParse::Instance().Parse(request.params.path, token)) {
        ServerLog::Warn("Failed to parse memory files.");
    }

    ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ClusterProcess, token, selectedFolder);
}

void ImportActionHandler::HandleCompute(ImportActionResponse &response, const std::string &selectedFolder) const
{
    Source::SourceFileParser &sourceFileParser = Source::SourceFileParser::Instance();
    sourceFileParser.Parse(std::vector<std::string>(), "", selectedFolder);
    sourceFileParser.ConvertToData();

    SetResponseResult(response, true);
    response.body.isBinary= true;
    response.body.coreList= sourceFileParser.GetCoreList();
    response.body.sourceList= sourceFileParser.GetSourceList();
}

void ImportActionHandler::ClusterProcess(const std::string &token, const std::string &selectedFolder)
{
    std::string parseClusterResult = "none";
    if (ImportActionHandler::curIsCluster) {
        ClusterFileParser clusterFileParser;
        if (clusterFileParser.ParseClusterFiles(selectedFolder)) {
            ServerLog::Info("ParseClusterFiles is success");
            parseClusterResult = "ok";
        } else {
            ServerLog::Warn("ParseClusterFiles is failed");
            parseClusterResult = "fail";
        }
    }
    // send event
    ImportActionHandler::ParseClusterEndProcess(token, parseClusterResult);
}

void ImportActionHandler::SetBaseActionOfResponse(const std::map<std::string, std::vector<std::string>> &rankListMap,
                                                  ImportActionResponse &response)
{
    for (const auto &rankEntry : rankListMap) {
        std::string rankId = rankEntry.first;
        Action action;
        action.cardName = rankId;
        action.rankId = rankId;
        action.result = true;
        std::string path = FileUtil::GetParentPath(rankEntry.second[0]);
                // 将文件所在路径的三级目录名称作为rank的tooltip信息
        action.cardPath = "Directory: " + FileUtil::GetRankIdFromPath(rankEntry.second[0]);
        response.body.result.emplace_back(action);
    }
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

void ImportActionHandler::ParseEndCallBack(const std::string &token, const std::string &fileId, bool result,
                                           const std::string &message)
{
    ServerLog::Info("Parse end, token = ", StringUtil::AnonymousString(token), " fileId:", fileId, ", result:", result);
    if (result) {
        SendParseSuccessEvent(token, fileId);
    } else {
        SendParseFailEvent(token, fileId, message);
    }
}

void ImportActionHandler::SendParseSuccessEvent(const std::string &token, const std::string &fileId)
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

void ImportActionHandler::SendParseFailEvent(const std::string &token, const std::string &fileId,
                                             const std::string &message)
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
    std::vector<std::string> traceFiles = {};
    if (!FileUtil::IsFolder(path)) {
        size_t length = JSON_FILE_SUFFIX.size();
        if (path.size() > length && path.substr(path.size() - length) == JSON_FILE_SUFFIX) {
            traceFiles.emplace_back(path);
        }
        return traceFiles;
    }
    std::function<void(const std::string&, int)> find = [&find, this, &traceFiles](const std::string &path, int depth) {
        if (depth > 5) {
            return;
        }
        std::vector<std::string> folders;
        std::vector<std::string> files;
        if (!FileUtil::FindFolders(path, folders, files)) {
            return;
        }
        if (std::find(folders.begin(), folders.end(), ASCEND_PROFILER_OUTPUT) != folders.end()) {
            curScene = "train";
            FindAscendFolder(path, traceFiles);
            return;
        }
        if (std::find(folders.begin(), folders.end(), MINDSTUDIO_PROFILER_OUTPUT) != folders.end()) {
            std::string tmpPath = FileUtil::SplicePath(path, MINDSTUDIO_PROFILER_OUTPUT);
            if (FileUtil::IsFolder(tmpPath)) {
                curScene = "infer";
                find(tmpPath, depth + 1);
                return;
            }
        }

        for (const auto &folder : folders) {
            std::string tmpPath = FileUtil::SplicePath(path, folder);
            find(tmpPath, depth + 1);
        }
        for (const auto &file : files) {
            if (IsJsonValid(file)) {
                traceFiles.push_back(FileUtil::SplicePath(path, file));
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
    std::string traceFilePath = FileUtil::SplicePath(path, ASCEND_PROFILER_OUTPUT);
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
        std::vector<std::string> folders;
        std::vector<std::string> files;
        if (!FileUtil::FindFolders(path, folders, files)) {
            return;
        }
        for (const auto &folder : folders) {
            std::string tmpPath = FileUtil::SplicePath(path, folder);
            find(tmpPath, depth + 1);
        }
        for (const auto &file : files) {
            if (IsJsonValid(file)) {
                traceFiles.push_back(FileUtil::SplicePath(path, file));
            }
        }
    };
    std::vector<std::string> folders;
    std::vector<std::string> files;
    if (!FileUtil::FindFolders(path, folders, files)) {
        return;
    }
    static std::string reg = R"(PROF_.*)";
    for (const auto &folder : folders) {
        if (!RegexUtil::RegexMatch(folder, reg).has_value()) {
            continue;
        }
        std::string tmpPath = FileUtil::SplicePath(path, folder);
        find(tmpPath, 0);
        break;
    }
}

void ImportActionHandler::SetParseCallBack(const std::string &token)
{
    std::function<void(const std::string, bool, const std::string)> func =
        std::bind(ParseEndCallBack, token, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    TraceFileParser::Instance().SetParseEndCallBack(func);
}

void ImportActionHandler::SearchMetaData(const std::string &fileId,
                                         std::vector<std::unique_ptr<UnitTrack>> &metaData)
{
    auto database = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", fileId);
        return;
    }
    database->QueryUnitsMetadata(fileId, metaData);
}

std::string ImportActionHandler::GetFileId(const std::string &filePath)
{
    std::string fileId = FileUtil::GetRankIdFromFile(filePath);
    int i = 1;
    std::string result = fileId;
    std::string parentDir = FileUtil::GetParentPath(filePath);
    std::string name = FileUtil::GetFileName(filePath);
    while (DataBaseManager::Instance().HasFileId(DatabaseType::TRACE, result)) {
        std::string dir = FileUtil::GetParentPath(DataBaseManager::Instance().GetTraceDatabase(result)->GetDbPath());
        if (RegexUtil::RegexMatch(name, MSPROF_SLICE_FILE_REG) && parentDir == dir) {
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

bool ImportActionHandler::CheckIsCluster(const std::string &filePath)
{
    std::vector<std::string> folders;
    std::vector<std::string> files;
    if (filePath.find(CLUSTER_ANALYSIS_OUTPUT) != std::string::npos) {
        ServerLog::Info("this folder is cluster_analysis_output,CheckIsCluster is true");
        return true;
    }
    if (!FileUtil::FindFolders(filePath, folders, files)) {
        ServerLog::Info("FindFolders is empty,CheckIsCluster is false");
        return false;
    }
    return std::any_of(folders.begin(), folders.end(),
                       [](std::string &folder) {return folder == CLUSTER_ANALYSIS_OUTPUT;});
}

std::vector<std::pair<std::string, std::string>> ImportActionHandler::GetTraceFiles(
    const std::vector<std::string> &pathList, ImportActionResBody &body)
{
    auto traceFiles = FindAllTraceFile(pathList);
    if (pathList.size() == 1) {
        bool isCluster = (traceFiles.size() > 1 && std::strcmp(curScene.c_str(), "train") == 0)
                || CheckIsCluster(pathList[0]);
        bool reset = isCluster || curIsCluster;
        ServerLog::Info("new Cluster:", isCluster, ", old Cluster:", curIsCluster, ", reset:", reset);
        curIsCluster = isCluster;
        if (reset) {
            TraceFileParser::Instance().Reset();
            Summary::KernelParse::Instance().Reset();
            body.reset = reset;
        }
        body.isCluster = isCluster;
    } else {
        body.isCluster = false;
    }
    if (traceFiles.empty()) {
        ServerLog::Warn("Failed to find trace file.");
        return {};
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

std::string ImportActionHandler::GetDbPath(const std::string &filePath, const int index)
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
} // Timeline
} // Module
} // Dic