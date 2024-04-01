/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "ParserJson.h"

#include <utility>
#include "FileUtil.h"
#include "TimelineRequestHandler.h"
#include "WsSession.h"
#include "WsSessionManager.h"
#include "ModuleRequestHandler.h"
#include "TraceFileParser.h"
#include "ModuleRequestHandler.h"
#include "ClusterParseThreadPoolExecutor.h"
#include "ClusterFileParser.h"
#include "DataBaseManager.h"
#include "MemoryParse.h"
#include "ParserStatusManager.h"
#include "EventNotifyThreadPoolExecutor.h"

namespace Dic {
namespace Module {
using namespace Timeline;
ParserJson::ParserJson() {}

ParserJson::~ParserJson() {}

void ParserJson::Parser(const std::string &path, ImportActionRequest &request)
{
    std::string token = request.token;
    Server::WsSession &session = *Server::WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr.get();
    ModuleRequestHandler::SetBaseResponse(request, response);

    Timeline::DataBaseManager::Instance().SetDataType(Timeline::DataType::JSON);
    auto files = GetTraceFiles(path, response.body);
    std::map<std::string, std::vector<std::string>> rankListMap = FileUtil::SplitToRankList(files);
    for (const auto &rankEntry : rankListMap) {
        SetBaseActionOfResponse(response, rankEntry);
    }
    SetParseCallBack(token);
    ModuleRequestHandler::SetResponseResult(response, true);
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = Protocol::ModuleType::TIMELINE;
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    for (const auto &rankEntry : rankListMap) {
        Timeline::TraceFileParser::Instance().Parse(rankEntry.second, rankEntry.first, path);
    }

    if (!Summary::KernelParse::Instance().Parse(request.params.path, token)) {
        ServerLog::Warn("Failed to parse kernel files.");
    }

    if (!Memory::MemoryParse::Instance().Parse(request.params.path, token)) {
        ServerLog::Warn("Failed to parse memory files.");
    }

    Timeline::ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ClusterProcess, token, path);
    Timeline::EventNotifyThreadPoolExecutor::Instance().GetThreadPool()->AddTask(SendAllParseSuccess, token);
}

void ParserJson::SendAllParseSuccess(const std::string &token)
{
    while (!ParserStatusManager::Instance().IsAllFinished()) {
        ServerLog::Info("all parse not finished");
        const int sleepTime = 2000;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    }
    ServerLog::Info("Send all parse finished");
    WsSession *session = WsSessionManager::Instance().GetSession(token);
    if (session == nullptr) {
        ServerLog::Warn("Failed to get session token ");
        return;
    }
    auto event = std::make_unique<ParseClusterCompletedEvent>();
    ParserStatusManager::Instance().SetClusterParseStatus(ParserStatus::FINISH);
    event->moduleName = ModuleType::TIMELINE;
    event->token = token;
    event->result = true;
    event->body.parseResult = PARSE_RESULT_OK;
    event->body.isAllPageParsed = true;
    session->OnEvent(std::move(event));
}

void ParserJson::SetParseCallBack(std::string token)
{
    std::function<void(const std::string, bool, const std::string)> func =
        std::bind(ParseEndCallBack, token, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    TraceFileParser::Instance().SetParseEndCallBack(func);
}


void ParserJson::ClusterProcess(const std::string &token, const std::string &selectedFolder)
{
    std::string parseClusterResult = PARSE_RESULT_NONE;
    if (ParserAlloc::curIsCluster) {
        ClusterFileParser clusterFileParser;
        if (clusterFileParser.ParseClusterFiles(selectedFolder)) {
            ServerLog::Info("ParseClusterFiles is success");
            parseClusterResult = PARSE_RESULT_OK;
            ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ClusterProcessAsyncStep, token,
                selectedFolder);
        } else {
            ServerLog::Warn("ParseClusterFiles is failed");
            parseClusterResult = PARSE_RESULT_FAIL;
        }
    }
    // send event
    ParserAlloc::ParseClusterEndProcess(token, parseClusterResult);
}

void ParserJson::ClusterProcessAsyncStep(const std::string &token, const std::string &selectedFolder)
{
    std::string parseClusterResult;
    ClusterFileParser clusterFileParser;
    if (ParserStatusManager::Instance().GetClusterParserStatus() == ParserStatus::FINISH ||
        clusterFileParser.ParseClusterStep2Files(selectedFolder)) {
        ServerLog::Info("ParseClusterStep2Files is success");
        parseClusterResult = PARSE_RESULT_OK;
    } else {
        ServerLog::Warn("ParseClusterStep2Files is failed");
        parseClusterResult = PARSE_RESULT_FAIL;
    }
    // send event
    ServerLog::Info("Parse Cluster File end, send event");
    WsSession *session = WsSessionManager::Instance().GetSession(token);
    if (session == nullptr) {
        ServerLog::Warn("Failed to get session token ");
        return;
    }
    auto event = std::make_unique<ParseClusterStep2CompletedEvent>();
    event->moduleName = ModuleType::TIMELINE;
    event->token = token;
    event->result = true;
    event->body.parseResult = std::move(parseClusterResult);
    session->OnEvent(std::move(event));
}

std::vector<std::pair<std::string, std::string>> ParserJson::GetTraceFiles(const std::string &path,
    ImportActionResBody &body)
{
    std::vector<std::string> traceFiles = FindAllTraceFile(path);
    CheckIfClusterAndReset(path, traceFiles.size(), body, false);
    if (traceFiles.empty()) {
        Server::ServerLog::Warn("Failed to find trace file.");
        return {};
    }
    std::vector<std::pair<std::string, std::string>> files;
    for (const auto &file : traceFiles) {
        std::string fileId = GetFileId(file);
        if (fileId.empty()) {
            Server::ServerLog::Error("File id is empty. file:", file);
            continue;
        }
        files.emplace_back(file, fileId);
    }
    ServerLog::Info("Get trace files finish. file size:", files.size());
    return files;
}

std::vector<std::string> ParserJson::FindAllTraceFile(const std::string &path)
{
    std::vector<std::string> traceFiles;
    if (path == "browser") {
        return FindTraceFile(ExecUtil::SelectFolder());
    }
    auto files = FindTraceFile(path);
    if (files.empty()) {
        ServerLog::Warn("Can't find trace file in path:", path);
    }
    traceFiles.insert(traceFiles.end(), files.begin(), files.end());
    return traceFiles;
}

std::vector<std::string> ParserJson::FindTraceFile(const std::string &path)
{
    std::vector<std::string> traceFiles = {};
    if (!FileUtil::IsFolder(path)) {
        size_t length = JSON_FILE_SUFFIX.size();
        if (path.size() > length && path.substr(path.size() - length) == JSON_FILE_SUFFIX) {
            traceFiles.emplace_back(path);
        }
        return traceFiles;
    }
    std::function<void(const std::string &, int)> find = [&find, this, &traceFiles](const std::string &path,
        int depth) {
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

bool ParserJson::IsJsonValid(const std::string &fileName)
{
    static std::string reg = R"(^(trace_view|msprof(_slice)?(_[0-9]{1,15}){1,4})\.json$)";
    auto result = RegexUtil::RegexMatch(fileName, reg);
    return result.has_value();
}

void ParserJson::FindAscendFolder(const std::string &path, std::vector<std::string> &traceFiles)
{
    std::string traceFilePath = FileUtil::SplicePath(path, ASCEND_PROFILER_OUTPUT);
    traceFilePath = FileUtil::SplicePath(traceFilePath, "trace_view.json");
    ServerLog::Info("FindAscendFolder. ", traceFilePath);
    if (FileUtil::CheckDirectoryExist(traceFilePath)) {
        traceFiles.emplace_back(traceFilePath);
        return;
    }
    std::function<void(const std::string &, int)> find = [&find, this, &traceFiles](const std::string &path,
        int depth) {
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
    static std::string reg = R"(PROF_[_\d\w]{0,64})";
    for (const auto &folder : folders) {
        if (!RegexUtil::RegexMatch(folder, reg).has_value()) {
            continue;
        }
        std::string tmpPath = FileUtil::SplicePath(path, folder);
        find(tmpPath, 0);
        break;
    }
}
} // Module
} // Dic