/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "ParserJson.h"
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
#include "BaselineManager.h"
#include "ProjectExplorerManager.h"
#include "TraceTime.h"
#include "FileReader.h"
#include "ParserJson.h"

namespace Dic {
namespace Module {
using namespace Timeline;
ParserJson::ParserJson()
{
    fileReader = std::make_unique<FileReader>();
}

ParserJson::~ParserJson() {}

void ParserJson::Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request)
{
    Timeline::DataBaseManager::Instance().SetDataType(Timeline::DataType::TEXT);
    if (projectInfos[0].importType == "drag") {
        ReloadDbPath(projectInfos, request);
        return;
    }
    // 基础信息填充
    Server::WsSession &session = *Server::WsSessionManager::Instance().GetSession();
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr.get();
    FillBaseResponseInfo(request, response, projectInfos);

    // 获取rankid及文件映射关系信息
    std::map<std::string, std::vector<std::string>> rankToFoldersMap;
    std::map<std::string, std::vector<std::string>> rankListMap = GetRankListMap(projectInfos, rankToFoldersMap);
    // 设置基础响应内容
    for (const auto &rankEntry : rankListMap) {
        auto folders = rankToFoldersMap[rankEntry.first];
        if (rankEntry.second.empty()) {
            continue;
        }
        std::string cardPath = FileUtil::GetRankIdFromPath(rankEntry.second[0]);
        SetBaseActionOfResponse(response, rankEntry.first, cardPath, folders);
    }
    // 解析内容
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
        TraceTime::Instance().SetIsSimulation(true);
        return;
    } else if (projectTypeEnum == ProjectTypeEnum::TEXT_CLUSTER) {
        response.body.isCluster = true;
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

void ParserJson::FillBaseResponseInfo(const ImportActionRequest &request, ImportActionResponse &response,
                                      const std::vector<Global::ProjectExplorerInfo> &projectInfos)
{
    ModuleRequestHandler::SetBaseResponse(request, response);
    std::vector<std::string> subdirectoryList = {};
    ComputeSubirectoryList(projectInfos, subdirectoryList);
    response.body.subdirectoryList = subdirectoryList;
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = MODULE_TIMELINE;
    response.body.reset = IsNeedReset(request);
    if (response.body.reset) {
        ParserFactory::Reset();
    }
}

void ParserJson::ComputeSubirectoryList(const std::vector<Global::ProjectExplorerInfo> &projectInfos,
    std::vector<std::string> &subdirectoryList)
{
    for (const auto &projectInfo : projectInfos) {
        for (const auto &item : projectInfo.parseFilePathInfos) {
            subdirectoryList.push_back(item.parseFilePath);
        }
    }
}

std::map<std::string, std::vector<std::string>> ParserJson::GetRankListMap(
    const std::vector<Global::ProjectExplorerInfo> &projectInfos,
    std::map<std::string, std::vector<std::string>> &rankToFoldersMap)
{
    // 获取单卡文件，并根据单卡所在目录获取其单卡信息
    std::map<std::string, std::vector<std::string>> rankToTraceMap;
    for (const auto &project : projectInfos) {
        for (const auto &parseFileInfo : project.parseFilePathInfos) {
            std::vector<std::string> jsonFiles = GetJsonFileUnderFolder(parseFileInfo.parseFilePath);
            if (!CheckParseFileInfoSize(parseFileInfo, jsonFiles)) {
                continue;
            }
            std::string fileId = GetFileId(jsonFiles[0], parseFileInfo.parseFilePath);
            rankToFoldersMap[fileId].push_back(parseFileInfo.parseFilePath);
            rankToTraceMap[fileId] = jsonFiles;
        }
    }
    return rankToTraceMap;
}

bool ParserJson::CheckParseFileInfoSize(const Global::ParseFileInfo &parseFileInfo,
    vector<std::string> &jsonFiles) const
{
    if (jsonFiles.empty()) {
        return false;
    }
    if (jsonFiles.size() > JSON_FILE_COUNT_LIMIT) {
        ServerLog::Warn("The number of json fragments in the ",
            StringUtil::GetPrintAbleString(parseFileInfo.parseFilePath), " exceeds ",
            std::to_string(JSON_FILE_COUNT_LIMIT));
        return false;
    }
    int64_t jsonFileSize = 0;
    for (const auto &item : jsonFiles) {
        int64_t singleJsonFileSize = fileReader->GetFileSize(item);
        if (singleJsonFileSize > JSON_MAX_FILE_SIZE || singleJsonFileSize + jsonFileSize > JSON_MAX_FILE_SIZE) {
            ServerLog::Warn("The file size in the ", StringUtil::GetPrintAbleString(parseFileInfo.parseFilePath),
                            " exceeds ", std::to_string(JSON_MAX_FILE_SIZE));
            return false;
        }
        jsonFileSize += singleJsonFileSize;
    }
    return true;
}

std::vector<std::string> ParserJson::GetJsonFileUnderFolder(const std::string &path)
{
    std::vector<std::string> jsonFiles;
    if (!FileUtil::IsFolder(path)) {
        jsonFiles.emplace_back(path);
        return jsonFiles;
    }
    std::vector<std::string> folders;
    std::vector<std::string> files;
    if (!FileUtil::FindFolders(path, folders, files)) {
        return jsonFiles;
    }
    for (const auto &file : files) {
        if (IsJsonValid(file)) {
            std::string jsonFile = FileUtil::SplicePath(path, file);
            jsonFiles.emplace_back(jsonFile);
        }
    }
    return jsonFiles;
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

    auto projectTypeEnum = static_cast<ProjectTypeEnum>(projectInfos[0].projectType);
    Timeline::ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ClusterProcess,
        projectInfos[0].fileName, projectTypeEnum, dataPathToDbMap, projectInfos[0].projectName);
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
    response.moduleName = MODULE_TIMELINE;
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    for (const auto &item : projectInfos) {
        std::string fileId = FileUtil::PathPreprocess(item.fileName);
        if (item.dbPath.empty()) {
            ParseEndCallBack(item.fileName, false, "Failed to get db file. Please delete and upload again.");
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
    std::ifstream file = OpenReadFileSafely(filePath);
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


void ParserJson::ClusterProcess(const std::string &selectedFolder, ProjectTypeEnum projectType,
    std::map<std::string, std::vector<std::string>> &dataPathToDbMap, const std::string &projectName)
{
    std::string parseClusterResult = PARSE_RESULT_NONE;
    if (projectType == ProjectTypeEnum::TEXT_CLUSTER) {
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
        ServerLog::Warn("Failed to get session");
        return;
    }
    auto event = std::make_unique<ParseClusterStep2CompletedEvent>();
    event->moduleName = MODULE_TIMELINE;
    event->result = true;
    event->body.parseResult = std::move(parseClusterResult);
    session->OnEvent(std::move(event));
}

std::vector<std::string> ParserJson::FindAllTraceFile(const std::string &path, std::string &error)
{
    std::vector<std::string> traceFiles;
    if (path == "browser") {
        return FindTraceFile(ExecUtil::SelectFolder(), error);
    }
    auto files = FindTraceFile(path, error);
    if (files.empty()) {
        ServerLog::Warn("Can't find trace file");
    }
    traceFiles.insert(traceFiles.end(), files.begin(), files.end());
    return traceFiles;
}

std::vector<std::string> ParserJson::FindTraceFile(const std::string &path, std::string &error)
{
    std::vector<std::string> traceFiles = {};
    if (!FileUtil::CheckFilePathLength(path)) {
        error = " File path length is limit " + std::to_string(FileUtil::GetFilePathLengthLimit()) +
            ",please shorten it!";
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
    if (!std::empty(error)) {
        return;
    }
    if (!FileUtil::IsWithinRecursionLimit(traceFiles, depth, error)) {
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
    // 检查traceFilePath是否存在且合法
    if (FileUtil::CheckDirValid(traceFilePath)) {
        traceFiles.emplace_back(traceFilePath);
        return;
    }
    std::string error;
    std::function<void(const std::string &, int)> find = [&find, this, &traceFiles, &error](const std::string &path,
        int depth) {
        if (!std::empty(error)) {
            return;
        }
        if (!FileUtil::IsWithinRecursionLimit(traceFiles, depth, error)) {
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
    if (!std::empty(error)) {
        ServerLog::Warn(StringUtil::GetPrintAbleString(path), " warn is: ", error);
    }
}

ProjectTypeEnum ParserJson::GetProjectType(const std::vector<std::string> &dataPath)
{
    std::string error;
    std::vector<std::string> traceFiles = FindAllTraceFile(dataPath[0], error);
    bool isCluster =
        (traceFiles.size() > 1 && std::strcmp(curScene.c_str(), "train") == 0) || CheckIsCluster(dataPath[0]);
    if (isCluster) {
        return ProjectTypeEnum::TEXT_CLUSTER;
    }
    if (!std::empty(traceFiles) && isSimulation(traceFiles[0])) {
        return ProjectTypeEnum::SIMULATION;
    }
    return ProjectTypeEnum::TRACE;
}

std::vector<std::string> ParserJson::GetParseFileByImportFile(const std::string &importFile,
    ProjectTypeEnum projectTypeEnum, std::string &error)
{
    // 如果是文件，直接返回
    if (!FileUtil::IsFolder(importFile)) {
        return { importFile };
    }
    // 分别获取trace、operator、memory文件
    auto traceFiles = FindAllTraceFile(importFile, error);
    auto opFiles = FileUtil::FindFilesWithFilter(importFile, std::regex(KERNEL_DETAIL_REG));
    auto memoryFiles = FileUtil::FindFilesWithFilter(importFile, std::regex(memoryRecordReg));

    // 将所有文件的父目录放到一个set集合中（利用set进行去重）
    std::set<std::string> resultSet;
    for (const auto &item : traceFiles) {
        resultSet.insert(FileUtil::GetParentPath(item));
    }
    for (const auto &item : opFiles) {
        resultSet.insert(FileUtil::GetParentPath(item));
    }
    for (const auto &item : memoryFiles) {
        resultSet.insert(FileUtil::GetParentPath(item));
    }

    if (resultSet.empty()) {
        return { importFile };
    }
    // 转换成vector返回
    std::vector<std::string> result(resultSet.begin(), resultSet.end());
    return result;
}

void ParserJson::ParserBaseline(const std::vector<Global::ProjectExplorerInfo> &projectInfos,
    Global::BaselineInfo &baselineInfo)
{
    if (projectInfos.empty() || projectInfos[0].parseFilePathInfos.empty()) {
        return;
    }
    // 当前只处理单卡情况
    std::string filePath = projectInfos[0].parseFilePathInfos[0].parseFilePath;
    std::vector<std::string> jsonFiles = GetJsonFileUnderFolder(filePath);
    if (std::empty(jsonFiles)) {
        return;
    }
    // 判断项目类型，如果是算子调优数据，则直接解析
    auto projectTypeEnum = static_cast<ProjectTypeEnum>(projectInfos[0].projectType);
    // 创建db连接池
    std::string dbPath = FileUtil::GetDbPath(jsonFiles[0]);
    std::map<std::string, std::vector<std::string>> rankToFoldersMap;
    std::map<std::string, std::vector<std::string>> rankListMap = GetRankListMap(projectInfos, rankToFoldersMap);
    if (std::empty(rankListMap)) {
        Global::BaselineManager::Instance().SetBaselineInfo(baselineInfo);
        baselineInfo.errorMessage = "Json get rank id failed!";
        return;
    }
    std::string rankId = rankListMap.begin()->first;
    baselineInfo.rankId = rankId;
    baselineInfo.cardName = "baseline" + rankId;
    Global::BaselineManager::Instance().SetBaselineInfo(baselineInfo);
    bool isParsed = DataBaseManager::Instance().IsContainDatabasePath(dbPath);
    if (!isParsed && !DataBaseManager::Instance().CreatConnectionPool(rankId, dbPath)) {
        ServerLog::Error("Failed to create connection pool. fileId:", rankId, ". path:", dbPath);
    }

    if (!isParsed && projectTypeEnum == ProjectTypeEnum::SIMULATION) {
        Timeline::TraceFileSimulationParser::Instance().Parse(jsonFiles, rankId, filePath);
        return;
    }

    // 如果是系统调优数据，分别解析trace、kernel和memory数据
    if (!isParsed &&
        !Timeline::TraceFileParser::Instance().Parse(jsonFiles, rankId, filePath)) {
        ServerLog::Warn("Failed to parse baseline trace files.");
    }

    if (!Summary::KernelParse::Instance().Parse(std::vector<std::string>(), rankId, filePath)) {
        ServerLog::Warn("Failed to parse baseline kernel files.");
    }

    if (!Memory::MemoryParse::Instance().Parse(std::vector<std::string>(), rankId, filePath)) {
        ServerLog::Warn("Failed to parse baseline memory files.");
    }
    Timeline::EventNotifyThreadPoolExecutor::Instance().GetThreadPool()->AddTask(SendAllParseSuccess);
}
} // Module
} // Dic