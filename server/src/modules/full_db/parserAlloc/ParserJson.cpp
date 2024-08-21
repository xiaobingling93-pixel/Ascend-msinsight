/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "TimelineRequestHandler.h"
#include "WsSession.h"
#include "WsSessionManager.h"
#include "ModuleRequestHandler.h"
#include "TraceFileParser.h"
#include "TraceFileSimulationParser.h"
#include "ClusterParseThreadPoolExecutor.h"
#include "ClusterFileParser.h"
#include "DataBaseManager.h"
#include "MemoryParse.h"
#include "ParserStatusManager.h"
#include "EventNotifyThreadPoolExecutor.h"
#include "ParserJson.h"

namespace Dic {
namespace Module {
using namespace Timeline;
ParserJson::ParserJson() {}

ParserJson::~ParserJson() {}

void ParserJson::Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request)
{
    Timeline::DataBaseManager::Instance().SetDataType(Timeline::DataType::TEXT);
    if (projectInfos[0].importType == "drag") {
        ReloadDbPath(projectInfos, request);
        return;
    }
    Server::WsSession &session = *Server::WsSessionManager::Instance().GetSession();
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr.get();
    ModuleRequestHandler::SetBaseResponse(request, response);
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = Protocol::ModuleType::TIMELINE;
    std::string error;
    std::map<std::string, std::vector<std::string>> rankListMap = GetRankListMap(response, projectInfos, error);
    if (!std::empty(error)) {
        SendParseFailEvent("", error);
    }
    auto projectTypeEnum = static_cast<ProjectTypeEnum>(projectInfos[0].projectType);
    if (projectTypeEnum == ProjectTypeEnum::SIMULATION) {
        SetParseCallBack(Timeline::TraceFileSimulationParser::Instance());
        ModuleRequestHandler::SetResponseResult(response, true);
        response.body.isSimulation = true;
        session.OnResponse(std::move(responsePtr));
        for (const auto &rankEntry : rankListMap) {
            Timeline::TraceFileSimulationParser::Instance().Parse(rankEntry.second, rankEntry.first,
                rankEntry.second[0]);
        }
        return;
    } else if (projectTypeEnum == ProjectTypeEnum::CLUSTER) {
        DataBaseManager::Instance().curIsCluster = true;
    }
    SetParseCallBack(Timeline::TraceFileParser::Instance());
    if (rankListMap.size() >= PENDIND_CRITICAL_VALUE) {
        response.body.isPending = true;
    }
    ModuleRequestHandler::SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    ParserTraceData(rankListMap, projectInfos, request);
}

std::map<std::string, std::vector<std::string>> ParserJson::GetRankListMap(ImportActionResponse &response,
    const std::vector<Global::ProjectExplorerInfo> &projectInfos, std::string &error)
{
    std::vector<std::pair<std::string, std::string>> files;
    std::vector<std::string> fileList;
    std::map<std::string, std::vector<std::string>> rankToSourceFileMap;
    for (const auto &item : projectInfos) {
        auto traceFiles = GetTraceFiles(item.fileName, response.body, error);
        for (const auto &traceFile : traceFiles) {
            rankToSourceFileMap[traceFile.second].push_back(item.fileName);
        }
        files.insert(files.end(), traceFiles.begin(), traceFiles.end());
        fileList.push_back(item.fileName);
    }
    std::map<std::string, std::vector<std::string>> rankListMap = FileUtil::SplitToRankList(files);
    for (const auto &rankEntry : rankListMap) {
        if (rankEntry.second.empty()) {
            continue;
        }
        std::string cardPath = FileUtil::GetRankIdFromPath(rankEntry.second[0]);
        SetBaseActionOfResponse(response, rankEntry.first, cardPath, rankToSourceFileMap[rankEntry.first]);
    }
    return rankListMap;
}

void ParserJson::ParserTraceData(const std::map<std::string, std::vector<std::string>> &rankListMap,
    const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request)
{
    std::vector<std::string> fileList;
    for (const auto &item : projectInfos) {
        fileList.push_back(item.fileName);
    }
    bool isParseTraceJson = rankListMap.size() < PENDIND_CRITICAL_VALUE;
    for (const auto &rankEntry : rankListMap) {
        if (!isParseTraceJson) {
            ParserStatusManager::Instance().SetPendingStatus(rankEntry.first,
                { ProjectTypeEnum::TRACE, rankEntry.second });
            continue;
        }
        Timeline::TraceFileParser::Instance().Parse(rankEntry.second, rankEntry.first, rankEntry.second[0]);
    }
    if (!Summary::KernelParse::Instance().Parse(fileList)) {
        ServerLog::Warn("Failed to parse kernel files.");
    }

    if (!Memory::MemoryParse::Instance().Parse(fileList)) {
        ServerLog::Warn("Failed to parse memory files.");
    }

    Timeline::ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ClusterProcess,
        projectInfos[0].fileName, dataPathToDbMap, projectInfos[0].projectName);
    Timeline::EventNotifyThreadPoolExecutor::Instance().GetThreadPool()->AddTask(SendAllParseSuccess);
}

void ParserJson::ReloadDbPath(const std::vector<Global::ProjectExplorerInfo> &projectInfos,
    const ImportActionRequest &request)
{
    if (projectInfos.empty()) {
        return;
    }
    Server::WsSession &session = *Server::WsSessionManager::Instance().GetSession();
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr.get();
    ModuleRequestHandler::SetBaseResponse(request, response);
    std::map<std::string, std::vector<std::string>> rankToSourceFileMap;
    for (const auto &item : projectInfos) {
        rankToSourceFileMap[item.fileName].push_back(item.fileName);
    }
    for (const auto &rankEntry : rankToSourceFileMap) {
        // 拖拽文件不存在文件路径信息，因此cardPath直接使用rankId
        SetBaseActionOfResponse(response, rankEntry.first, rankEntry.first, rankEntry.second);
    }
    ModuleRequestHandler::SetResponseResult(response, true);
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = Protocol::ModuleType::TIMELINE;
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    for (const auto &item : projectInfos) {
        std::string fileId = FileUtil::PathPreprocess(item.fileName);
        if (item.dbPath.empty()) {
            ParseEndCallBack(item.fileName, false,
                             "Failed to get db file. Please delete and upload again.");
        }
        if (DataBaseManager::Instance().HasFileId(DatabaseType::TRACE, fileId)) {
            return;
        }
        DataBaseManager::Instance().CreatConnectionPool(fileId, item.dbPath[0]);
        ParseEndCallBack(item.fileName, true, "");
    }
}

bool ParserJson::isSimulation(std::string filePath)
{
    std::ifstream file = FileUtil::OpenReadFileSafely(filePath);
    if (!file.is_open()) {
        return false;
    }
    std::string headerString;
    int64_t contentStart = 0;
    file.seekg(contentStart, std::ios::beg);
    char startTemp;
    while (file.get(startTemp)) {
        headerString += startTemp;
        if (startTemp == '[') {
            break;
        }
        contentStart++;
    }
    ServerLog::Info("Import first file header is: ", headerString);
    if (headerString.find("profilingType") != std::string::npos && headerString.find("op") != std::string::npos) {
        return true;
    }
    return false;
}

void ParserJson::SetParseCallBack(FileParser &fileParser)
{
    std::function<void(const std::string, bool, const std::string)> func =
        std::bind(ParseEndCallBack, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    fileParser.SetParseEndCallBack(func);

    // 复用解析完成回调函数设置逻辑
    std::function<void(const std::string, uint64_t parsedSize, uint64_t totalSize, int progress)> progressFunc =
        std::bind(ParseProgressCallBack, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
        std::placeholders::_4);
    fileParser.SetParseProgressCallBack(progressFunc);
}


void ParserJson::ClusterProcess(const std::string &selectedFolder,
    std::map<std::string, std::vector<std::string>> &dataPathToDbMap, const std::string &projectName)
{
    std::string parseClusterResult = PARSE_RESULT_NONE;
    if (DataBaseManager::Instance().curIsCluster) {
        ClusterFileParser clusterFileParser;
        if (clusterFileParser.ParseClusterFiles(selectedFolder)) {
            ServerLog::Info("The cluster file is parsed successfully.");
            parseClusterResult = PARSE_RESULT_OK;
            dataPathToDbMap[selectedFolder].push_back(clusterFileParser.GetClusterDbPath());
            ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ClusterProcessAsyncStep,
                                                                                selectedFolder);
        } else {
            ServerLog::Warn("Failed to parse cluster files.");
            parseClusterResult = PARSE_RESULT_FAIL;
        }
    }
    // send event
    ParserAlloc::ParseClusterEndProcess(parseClusterResult);
    SaveDbPath(projectName, dataPathToDbMap);
}

void ParserJson::ClusterProcessAsyncStep(const std::string &selectedFolder)
{
    std::string parseClusterResult;
    ClusterFileParser clusterFileParser;
    if (ParserStatusManager::Instance().GetClusterParserStatus() == ParserStatus::FINISH ||
        clusterFileParser.ParseClusterStep2Files(selectedFolder)) {
        ServerLog::Info("The cluster step2 file is parsed successfully.");
        parseClusterResult = PARSE_RESULT_OK;
    } else {
        ServerLog::Warn("Failed to parse cluster step2 files.");
        parseClusterResult = PARSE_RESULT_FAIL;
    }
    // send event
    ServerLog::Info("Parse Cluster File end, send event");
    WsSession *session = WsSessionManager::Instance().GetSession();
    if (session == nullptr) {
        ServerLog::Warn("Failed to get session token ");
        return;
    }
    auto event = std::make_unique<ParseClusterStep2CompletedEvent>();
    event->moduleName = ModuleType::TIMELINE;
    event->result = true;
    event->body.parseResult = std::move(parseClusterResult);
    session->OnEvent(std::move(event));
}

std::vector<std::pair<std::string, std::string>> ParserJson::GetTraceFiles(const std::string &path,
    ImportActionResBody &body, std::string &error)
{
    std::vector<std::string> traceFiles = FindAllTraceFile(path, error);
    CheckIfClusterAndReset(path, traceFiles.size(), body, false);
    if (traceFiles.empty()) {
        Server::ServerLog::Warn("Failed to find trace file.");
        return {};
    }
    std::vector<std::pair<std::string, std::string>> files;
    for (const auto &file : traceFiles) {
        std::string fileId = GetFileId(file, path);
        if (fileId.empty()) {
            Server::ServerLog::Error("File id is empty. file:", file);
            continue;
        }
        files.emplace_back(file, fileId);
    }
    ServerLog::Info("Get trace files finish. file size:", files.size());
    return files;
}

std::vector<std::string> ParserJson::FindAllTraceFile(const std::string &path, std::string &error)
{
    std::vector<std::string> traceFiles;
    if (path == "browser") {
        return FindTraceFile(ExecUtil::SelectFolder(), error);
    }
    auto files = FindTraceFile(path, error);
    if (files.empty()) {
        ServerLog::Warn("Can't find trace file in path:", path);
    }
    traceFiles.insert(traceFiles.end(), files.begin(), files.end());
    return traceFiles;
}

std::vector<std::string> ParserJson::FindTraceFile(const std::string &path, std::string &error)
{
    std::vector<std::string> traceFiles = {};
    if (!FileUtil::CheckFilePathLength(path)) {
        error = " File path length is limit " + std::to_string(FileUtil::GetFilePathLengthLimit()) +
            ",please shorten it: " + path;
        return traceFiles;
    }
    if (!FileUtil::IsFolder(path)) {
        size_t length = JSON_FILE_SUFFIX.size();
        if (path.size() > length && path.substr(path.size() - length) == JSON_FILE_SUFFIX) {
            traceFiles.emplace_back(path);
        }
        return traceFiles;
    }
    FindTraceFiles(path, 0, error, traceFiles);
    return traceFiles;
}

void ParserJson::FindTraceFiles(const std::string &path, int depth, std::string &error,
    std::vector<std::string> &traceFiles)
{
    const int maxDepth = 5;
    if (depth > maxDepth) {
        return;
    }
    if (!std::empty(error)) {
        return;
    }
    if (!FileUtil::CheckFilePathLength(path)) {
        error = " File path length is limit " + std::to_string(FileUtil::GetFilePathLengthLimit()) +
            ",please shorten it: " + path;
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
            FindTraceFiles(tmpPath, depth + 1, error, traceFiles);
            return;
        }
    }

    for (const auto &folder : folders) {
        std::string tmpPath = FileUtil::SplicePath(path, folder);
        FindTraceFiles(tmpPath, depth + 1, error, traceFiles);
    }
    for (const auto &file : files) {
        if (IsJsonValid(file)) {
            traceFiles.push_back(FileUtil::SplicePath(path, file));
        }
    }
}

bool ParserJson::IsJsonValid(const std::string &fileName)
{
    static std::string reg = R"(^(trace_view|trace|msprof(_slice)?(_[0-9]{1,15}){1,4})\.json$)";
    auto result = RegexUtil::RegexMatch(fileName, reg);
    return result.has_value();
}

void ParserJson::FindAscendFolder(const std::string &path, std::vector<std::string> &traceFiles)
{
    std::string traceFilePath = FileUtil::SplicePath(path, ASCEND_PROFILER_OUTPUT);
    traceFilePath = FileUtil::SplicePath(traceFilePath, "trace_view.json");
    ServerLog::Info("FindAscendFolder. ", traceFilePath);
    // 检查traceFilePath是否存在
    if (FileUtil::CheckDirAccess(traceFilePath, 0)) {
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

ProjectTypeEnum ParserJson::GetProjectType(const std::vector<std::string> &dataPath)
{
    std::string error;
    std::vector<std::string> traceFiles = FindAllTraceFile(dataPath[0], error);
    bool isCluster =
        (traceFiles.size() > 1 && std::strcmp(curScene.c_str(), "train") == 0) || CheckIsCluster(dataPath[0]);
    if (isCluster) {
        return ProjectTypeEnum::CLUSTER;
    }
    if (!std::empty(traceFiles) && isSimulation(traceFiles[0])) {
        return ProjectTypeEnum::SIMULATION;
    }
    return ProjectTypeEnum::TRACE;
}
} // Module
} // Dic