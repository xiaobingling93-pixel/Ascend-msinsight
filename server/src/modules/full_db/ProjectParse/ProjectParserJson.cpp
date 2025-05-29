/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "ProjectParserJson.h"
#include "TimelineRequestHandler.h"
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
#include "MetaDataParser.h"
#include "MetaDataCacheManager.h"
#include "TraceTime.h"
#include "TimeUtil.h"
#include "FileReader.h"
#include "ProjectAnalyze.h"

namespace Dic::Module {
using namespace Timeline;
using namespace Global;
ProjectParserJson::ProjectParserJson()
{
    fileReader = std::make_unique<FileReader>();
}


// LCOV_EXCL_BR_START
void ProjectParserJson::Parser(const std::vector<ProjectExplorerInfo> &projectInfos, ImportActionRequest &request)
{
    Timeline::DataBaseManager::Instance().SetDataType(Timeline::DataType::TEXT);
    // 基础信息填充
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr;
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
        std::string fileId = GetFileIdWithDb(folders[0]);
        SetBaseActionOfResponse(response, rankEntry.first, fileId, cardPath, folders);
    }
    // 解析内容
    auto projectTypeEnum = Global::ProjectExplorerManager::GetProjectType(projectInfos);
    if (projectTypeEnum == ProjectTypeEnum::SIMULATION) {
        SetParseCallBack(Timeline::TraceFileSimulationParser::Instance());
        response.body.isSimulation = true;
        auto [hasTraceJson, hasMemoryData, hasOperatorData] = CheckHasTraceJsonMemoryDataOperatorData(projectInfos);
        response.body.isOnlyTraceJson = hasTraceJson && !hasMemoryData && !hasOperatorData;
        SendResponse(std::move(responsePtr), true);
        for (const auto &rankEntry : rankListMap) {
            Timeline::TraceFileSimulationParser::Instance().Parse(rankEntry.second, rankEntry.first,
                                                                  rankEntry.second[0]);
        }
        TraceTime::Instance().SetIsSimulation(true);
        return;
    }
    bool isCluster = CheckIsOpenClusterTag(request.params.projectAction, projectTypeEnum,
                                           projectInfos[0].projectName);
    response.body.isCluster = isCluster;
    SetParseCallBack(Timeline::TraceFileParser::Instance());
    if (rankListMap.size() >= PENDIND_CRITICAL_VALUE) {
        response.body.isPending = true;
    }
    auto [hasTraceJson, hasMemoryData, hasOperatorData] = CheckHasTraceJsonMemoryDataOperatorData(projectInfos);
    response.body.isOnlyTraceJson = hasTraceJson && !hasMemoryData && !hasOperatorData && !isCluster;
    ModuleRequestHandler::SetResponseResult(response, true);
    // add response to response queue in session
    SendResponse(std::move(responsePtr), true);
    ParserTraceData(rankListMap, projectInfos, isCluster);
}
// LCOV_EXCL_BR_STOP

void ProjectParserJson::FillBaseResponseInfo(const ImportActionRequest &request, ImportActionResponse &response,
                                             const std::vector<ProjectExplorerInfo> &projectInfos)
{
    ModuleRequestHandler::SetBaseResponse(request, response);
    std::for_each(projectInfos.begin(), projectInfos.end(), [&response](const ProjectExplorerInfo &info) {
        std::copy(info.subParseFileInfo.begin(), info.subParseFileInfo.end(),
                  std::back_inserter(response.body.subParseFileInfo));
        std::copy(info.projectFileTree.begin(), info.projectFileTree.end(),
                  std::back_inserter(response.body.projectFileTree));
    });
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = MODULE_TIMELINE;
    response.body.reset = IsNeedReset(request);
    if (response.body.reset) {
        ParserFactory::Reset();
    }
}

std::map<std::string, std::vector<std::string>> ProjectParserJson::GetRankListMap(
    const std::vector<Global::ProjectExplorerInfo> &projectInfos,
    std::map<std::string, std::vector<std::string>> &rankToFoldersMap)
{
    // 获取单卡文件，并根据单卡所在目录获取其单卡信息
    std::map<std::string, std::vector<std::string>> rankToTraceMap;
    for (const auto &project : projectInfos) {
        for (const auto &parseFileInfo : project.subParseFileInfo) {
            std::vector<std::string> jsonFiles = GetJsonFileUnderFolder(parseFileInfo->parseFilePath);
            if (!CheckParseFileInfoSize(parseFileInfo, jsonFiles)) {
                continue;
            }
            std::string fileId = GetFileId(jsonFiles[0], parseFileInfo->parseFilePath);
            rankToFoldersMap[fileId].push_back(parseFileInfo->parseFilePath);
            rankToTraceMap[fileId] = jsonFiles;
        }
    }
    return rankToTraceMap;
}

bool ProjectParserJson::CheckParseFileInfoSize(const std::shared_ptr<Global::ParseFileInfo> &parseFileInfo,
    std::vector<std::string> &jsonFiles) const
{
    if (jsonFiles.empty()) {
        return false;
    }
    if (jsonFiles.size() > JSON_FILE_COUNT_LIMIT) {
        ServerLog::Warn("The number of json fragments in the ",
            StringUtil::GetPrintAbleString(parseFileInfo->parseFilePath), " exceeds ",
            std::to_string(JSON_FILE_COUNT_LIMIT));
        return false;
    }
    int64_t jsonFileSize = 0;
    for (const auto &item : jsonFiles) {
        int64_t singleJsonFileSize = fileReader->GetFileSize(item);
        if (singleJsonFileSize > JSON_MAX_FILE_SIZE || singleJsonFileSize + jsonFileSize > JSON_MAX_FILE_SIZE) {
            ServerLog::Warn("The file size in the ", StringUtil::GetPrintAbleString(parseFileInfo->parseFilePath),
                            " exceeds ", std::to_string(JSON_MAX_FILE_SIZE));
            return false;
        }
        jsonFileSize += singleJsonFileSize;
    }
    return true;
}

std::vector<std::string> ProjectParserJson::GetJsonFileUnderFolder(const std::string &path)
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

void ProjectParserJson::ParserTraceData(const std::map<std::string, std::vector<std::string>> &rankListMap,
    const std::vector<Global::ProjectExplorerInfo> &projectInfos, bool isShowCluster)
{
    std::vector<std::string> fileList;
    for (const auto &item : projectInfos) {
        fileList.push_back(item.fileName);
    }
    // 对metadata数据进行解析
    ParserMetaData(projectInfos);
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

    auto projectTypeEnum = Global::ProjectExplorerManager::GetProjectType(projectInfos);
    auto clusterInfos = Global::ProjectExplorerManager::GetClusterFilePath(projectInfos);
    if (clusterInfos.empty()) {
        std::for_each(projectInfos.begin(), projectInfos.end(), [&clusterInfos](const ProjectExplorerInfo& item) {
            auto cluster = std::make_shared<ParseFileInfo>();
            cluster->parseFilePath = item.fileName;
            cluster->type = ParseFileType::CLUSTER;
            cluster->clusterId = item.fileName;
            clusterInfos.emplace_back(cluster);
        });
    }
    auto clusterParse = [projectTypeEnum, isShowCluster, &projectInfos, this](const auto &item) {
        Timeline::ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(
            ProjectParserJson::ClusterProcess,
            item,
            projectTypeEnum,
            isShowCluster,
            dataPathToDbMap,
            projectInfos[0].projectName);
    };
    std::for_each(clusterInfos.begin(), clusterInfos.end(), clusterParse);
    Timeline::EventNotifyThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ParsePostProcess, clusterInfos);
}

bool ProjectParserJson::isSimulation(std::string filePath)
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

void ProjectParserJson::SetParseCallBack(FileParser &fileParser)
{
    std::function<void(const std::string, const std::string, bool, const std::string)> func =
        std::bind(ParseEndCallBack,
                  std::placeholders::_1,
                  std::placeholders::_2,
                  std::placeholders::_3,
                  std::placeholders::_4);
    fileParser.SetParseEndCallBack(func);

    // 复用解析完成回调函数设置逻辑
    std::function<void(const std::string, uint64_t parsedSize, uint64_t totalSize, int progress)> progressFunc =
        std::bind(ParseProgressCallBack, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
        std::placeholders::_4);
    fileParser.SetParseProgressCallBack(progressFunc);
}


void ProjectParserJson::ClusterProcess(std::shared_ptr<ParseFileInfo> clusterInfo, ProjectTypeEnum projectType,
                                       bool isShowCluster,
                                       std::map<std::string, std::vector<std::string>> &dataPathToDbMap,
                                       const std::string &projectName)
{
    if (clusterInfo == nullptr || clusterInfo->type != ParseFileType::CLUSTER || clusterInfo->parseFilePath.empty()) {
        ServerLog::Warn("Invalid cluster to parsed, end process");
        return;
    }
    std::string parseClusterResult = PARSE_RESULT_NONE;
    if (projectType == ProjectTypeEnum::TEXT_CLUSTER) {
        auto database = DataBaseManager::Instance().CreateClusterDatabase(clusterInfo->parseFilePath, DataType::TEXT);
        if (!database) {
            ServerLog::Warn("Failed to creat cluster db.path:" + clusterInfo->parseFilePath);
            return;
        }
        ClusterFileParser clusterFileParser(clusterInfo->parseFilePath, database,
                                            clusterInfo->clusterId + TimeUtil::Instance().NowStr());
        if (clusterFileParser.ParseClusterFiles()) {
            ServerLog::Info("The cluster file is parsed successfully.");
            parseClusterResult = PARSE_RESULT_OK;
            dataPathToDbMap[clusterInfo->parseFilePath].push_back(clusterFileParser.GetClusterDbPath());
            ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ClusterProcessAsyncStep,
                                                                                clusterFileParser);
        } else {
            ServerLog::Warn("Failed to parse cluster files.");
            parseClusterResult = PARSE_RESULT_FAIL;
        }
    }
    // send event
    ProjectParserBase::ParseClusterEndProcess(parseClusterResult, isShowCluster, clusterInfo->parseFilePath);
    SaveDbPath(projectName, dataPathToDbMap);
}

void ProjectParserJson::ClusterProcessAsyncStep(ClusterFileParser clusterFileParser)
{
    std::string parseClusterResult;
    if (clusterFileParser.ParseClusterStep2Files()) {
        ServerLog::Info("The cluster step2 file is parsed successfully.");
        parseClusterResult = PARSE_RESULT_OK;
    } else {
        ServerLog::Warn("Failed to parse cluster step2 files.");
        parseClusterResult = PARSE_RESULT_FAIL;
    }
    // send event
    ServerLog::Info("Parse Cluster File end, send event");
    auto event = std::make_unique<ParseClusterStep2CompletedEvent>();
    event->moduleName = MODULE_TIMELINE;
    event->result = true;
    event->body.clusterPath = clusterFileParser.GetClusterPath();
    event->body.parseResult = std::move(parseClusterResult);
    SendEvent(std::move(event));
}

std::vector<std::string> ProjectParserJson::FindAllTraceFile(const std::string &path, std::string &error)
{
    std::vector<std::string> traceFiles;
    if (path == "browser") {
        return FindTraceFile(ExecUtil::SelectFolder(), error, curScene);
    }
    auto files = FindTraceFile(path, error, curScene);
    if (files.empty()) {
        ServerLog::Warn("Can't find trace file");
    }
    traceFiles.insert(traceFiles.end(), files.begin(), files.end());
    return traceFiles;
}

std::vector<std::string> ProjectParserJson::FindTraceFile(const std::string &path, std::string &error,
                                                          std::string &curScene)
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
    FindTraceFiles(path, 0, error, traceFiles, curScene);
    return traceFiles;
}

void ProjectParserJson::FindTraceFiles(const std::string &path, int depth, std::string &error,
                                       std::vector<std::string> &traceFiles,
                                       std::string &curScene)
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
            FindTraceFiles(tmpPath, depth + 1, error, traceFiles, curScene);
            return;
        }
    }

    for (const auto &folder : folders) {
        std::string tmpPath = FileUtil::SplicePath(path, folder);
        FindTraceFiles(tmpPath, depth + 1, error, traceFiles, curScene);
    }
    for (const auto &file : files) {
        if (IsJsonValid(file)) {
            traceFiles.push_back(FileUtil::SplicePath(path, file));
        }
    }
}

bool ProjectParserJson::IsJsonValid(const std::string &fileName)
{
    static std::string reg = R"(^(trace_view|trace|msprof(_slice)?(_[0-9]{1,15}){1,4})\.json$)";
    auto result = RegexUtil::RegexMatch(fileName, reg);
    return result.has_value();
}

// LCOV_EXCL_BR_START
void ProjectParserJson::FindAscendFolder(const std::string &path, std::vector<std::string> &traceFiles)
{
    std::string traceFilePath = FileUtil::SplicePath(path, ASCEND_PROFILER_OUTPUT);
    traceFilePath = FileUtil::SplicePath(traceFilePath, "trace_view.json");
    // 检查traceFilePath是否存在且合法
    if (FileUtil::CheckDirValid(traceFilePath)) {
        traceFiles.emplace_back(traceFilePath);
        return;
    }
    std::string error;
    std::function<void(const std::string &, int)> find = [&find, &traceFiles, &error](const std::string &path,
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

ProjectTypeEnum ProjectParserJson::GetProjectType(const std::string &dataPath)
{
    std::string error;
    std::vector<std::string> traceFiles = FindAllTraceFile(dataPath, error);
    bool isCluster = (traceFiles.size() > 1 && (curScene == "train" || curScene == "infer")) ||
                     ClusterFileParser::CheckIsCluster(dataPath);
    if (isCluster) {
        return ProjectTypeEnum::TEXT_CLUSTER;
    }
    if (!std::empty(traceFiles) && isSimulation(traceFiles[0])) {
        return ProjectTypeEnum::SIMULATION;
    }
    return ProjectTypeEnum::TRACE;
}

std::vector<std::string> ProjectParserJson::GetParseFileByImportFile(const std::string &importFile, std::string &error)
{
    // 如果是文件，直接返回
    if (!FileUtil::IsFolder(importFile)) {
        return { importFile };
    }
    // 分别获取trace、operator、memory文件
    auto traceFiles = FindAllTraceFile(importFile, error);
    auto opFiles = FileUtil::FindFilesWithFilter(importFile, std::regex(KERNEL_DETAIL_REG));
    auto memoryFiles = FileUtil::FindFilesWithFilter(importFile, std::regex(memoryRecordReg));
    if (traceFiles.empty() && opFiles.empty() && memoryFiles.empty()) {
        error = "No parsable text files found, Possible reasons:; 1.File not exist; "
                "2.The nesting depth of the imported sub-file exceeds 5; 3.The sub-file path length exceeds " +
                std::to_string(FileUtil::GetFilePathLengthLimit());
        ServerLog::Info(error);
        return { importFile };
    }
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

void ProjectParserJson::ParserClusterBaseline(const Global::ProjectExplorerInfo &projectInfo)
{
    // 创建新的db连接对象
    std::string clusterPath = BaselineManager::Instance().GetBaseLineClusterPath();
    std::string uniqueKey = FileUtil::GetFileName(clusterPath) + TimeUtil::Instance().NowStr();
    // 创建新的db连接对象
    auto database = DataBaseManager::Instance().CreateClusterDatabase(
        BaselineManager::Instance().GetBaseLineClusterPath(), DataType::TEXT);
    // 集群解析，如果集群已解析，则只会初始化db，然后结束流程
    ClusterFileParser clusterFileParser(clusterPath, database, uniqueKey);
    if (clusterFileParser.ParseClusterFiles()) {
        ServerLog::Info("The cluster file is parsed successfully.");
        ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(
            [](ClusterFileParser parser) -> bool {
                return parser.ParseClusterStep2Files();
            },
            clusterFileParser);
    }
}
// LCOV_EXCL_BR_STOP

void ProjectParserJson::ParserSingleCardBaseline(const Global::ProjectExplorerInfo &projectInfos,
                                                 Global::BaselineInfo &baselineInfo)
{
    std::string filePath = projectInfos.subParseFileInfo[0]->parseFilePath;
    std::vector<std::string> jsonFiles = GetJsonFileUnderFolder(filePath);
    if (std::empty(jsonFiles)) {
        return;
    }
    // 判断项目类型，如果是算子调优数据，则直接解析
    auto projectTypeEnum = static_cast<ProjectTypeEnum>(projectInfos.projectType);
    // 创建db连接池
    std::string dbPath = FileUtil::GetDbPath(jsonFiles[0]);
    std::map<std::string, std::vector<std::string>> rankToFoldersMap;
    std::map<std::string, std::vector<std::string>> rankListMap = GetRankListMap({projectInfos}, rankToFoldersMap);
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

void ProjectParserJson::ParserBaseline(const Global::ProjectExplorerInfo &projectInfo,
    Global::BaselineInfo &baselineInfo)
{
    if (projectInfo.fileInfoMap.empty()) {
        return;
    }
    // 判断是否为集群
    if (baselineInfo.isCluster) {
        ParserClusterBaseline(projectInfo);
    } else {
        ParserSingleCardBaseline(projectInfo, baselineInfo);
    }
}

void ProjectParserJson::ParserMetaData(const std::vector<Global::ProjectExplorerInfo> &projectInfos)
{
    for (const auto &project: projectInfos) {
        for (const auto &item: project.subParseFileInfo) {
            std::string parent = FileUtil::GetParentPath(item->parseFilePath);
            std::string metaDataFilePath = FileUtil::SplicePath(parent, PROFILER_METADATA_FILE);
            if (!FileUtil::CheckDirValid(metaDataFilePath)) {
                ServerLog::Error("Meta data file % is not valid.", metaDataFilePath);
                continue;
            }
            auto groupInfoList = MetaDataParser::ParserParallelGroupInfoByFilePath(metaDataFilePath);
            MetaDataCacheManager::Instance().AddParallelGroupInfo(groupInfoList);
            if (MetaDataCacheManager::Instance().GetDistributedArgsInfo() != std::nullopt) {
                continue;
            }
            std::optional<DistributedArgs> args = MetaDataParser::ParserDistributedArgsByFilePath(metaDataFilePath);
            MetaDataCacheManager::Instance().SetDistributedArgsInfo(args);
        }
    }
}

bool ProjectParserJson::ExistJsonFormatFile(const std::string &file)
{
    if (file.empty()) {
        return false;
    }
    std::string error;
    std::string select = (file == "browser") ? ExecUtil::SelectFolder() : file;
    std::string scene;
    auto traceFiles = FindTraceFile(file, error, scene);
    auto opFiles = FileUtil::FindFilesWithFilter(file, std::regex(KERNEL_DETAIL_REG));
    auto memoryFiles = FileUtil::FindFilesWithFilter(file, std::regex(memoryRecordReg));
    if (traceFiles.empty() && opFiles.empty() && memoryFiles.empty()) {
        error = "Not find valid json text dir!";
        ServerLog::Info(error);
        return false;
    }
    return true;
}

std::tuple<bool, bool, bool> ProjectParserJson::CheckHasTraceJsonMemoryDataOperatorData(
    const std::vector<Global::ProjectExplorerInfo> &projectInfos)
{
    static const std::regex memoryRegex(memoryRecordReg);
    static const std::regex kernelRegex(KERNEL_DETAIL_REG);

    bool hasTraceJson = false;
    bool hasMemoryCsv = false;
    bool hasOperatorCsv = false;

    for (const auto &project : projectInfos) {
        if (hasTraceJson && hasMemoryCsv && hasOperatorCsv) break; // 提前退出

        for (const auto &item : project.subParseFileInfo) {
            std::string fileName = FileUtil::GetFileName(item->parseFilePath);
            if (!hasTraceJson && StringUtil::EndWith(fileName, JSON_FILE_SUFFIX)) {
                hasTraceJson = true;
            } else if (!hasMemoryCsv && std::regex_match(fileName, memoryRegex)) {
                hasMemoryCsv = true;
            } else if (!hasOperatorCsv && std::regex_match(fileName, kernelRegex)) {
                hasOperatorCsv = true;
            }

            if (hasTraceJson && hasMemoryCsv && hasOperatorCsv) break; // 内层循环提前退出
        }
    }

    return {hasTraceJson, hasMemoryCsv, hasOperatorCsv}; // 使用列表初始化
}

void ProjectParserJson::BuildProjectExploreInfo(ProjectExplorerInfo &info, const std::vector<std::string> &parsedFiles)
{
    ProjectParserBase::BuildProjectExploreInfo(info, parsedFiles);
    std::for_each(parsedFiles.begin(), parsedFiles.end(), [&info](const std::string &file) {
        ProjectParserJson::BuildProjectFromParseFile(info, file);
    });
}

void ProjectParserJson::BuildProjectFromParseFile(ProjectExplorerInfo &info, const std::string &parseFile)
{
    std::vector<std::string> parentFolders = GetParentFileList(info.fileName, parseFile);
    // Json工程层次：project-cluster-host-rank
    auto parseFileInfoRank = std::make_shared<ParseFileInfo>();
    parseFileInfoRank->parseFilePath = parseFile;
    parseFileInfoRank->type = ParseFileType::RANK;
    parseFileInfoRank->subId = parseFile;
    parseFileInfoRank->curDirName = FileUtil::GetFileName(parseFile);
    parseFileInfoRank->fileId = GetFileIdWithDb(parseFile);
    // import single file
    if (FileUtil::IsRegularFile(parseFile) || parseFileInfoRank->subId == info.fileName) {
        parseFileInfoRank->subId = FileUtil::GetFileName(parseFile);
        info.AddSubParseFileInfo(info.fileName, ParseFileType::PROJECT, parseFileInfoRank);
        return;
    }
    // 设置cluster信息
    std::string cluster;
    std::string clusterPrefix;
    constexpr uint64_t clusterFolderCount = 2;
    if (parentFolders.size() >= clusterFolderCount) {
        std::tie(cluster, clusterPrefix) = GetClusterInfo(parentFolders);
        if (info.GetSubParseFileInfo(clusterPrefix, ParseFileType::CLUSTER) == nullptr) {
            auto clusterInfo = std::make_shared<ParseFileInfo>();
            clusterInfo->subId = clusterPrefix;
            clusterInfo->type = ParseFileType::CLUSTER;
            clusterInfo->clusterId = FileUtil::GetFileName(cluster);
            clusterInfo->parseFilePath = clusterPrefix;
            clusterInfo->curDirName = FileUtil::GetFileName(cluster);
            info.AddSubParseFileInfo(info.fileName, ParseFileType::PROJECT, clusterInfo);
        }
        parentFolders.erase(parentFolders.begin());
    }

    if (clusterPrefix.empty()) {
        info.AddSubParseFileInfo(info.fileName, ParseFileType::PROJECT, parseFileInfoRank);
    } else {
        info.AddSubParseFileInfo(clusterPrefix, ParseFileType::CLUSTER, parseFileInfoRank);
    }
}

std::string ProjectParserJson::GetFileIdWithDb(const std::string &filePath)
{
    std::string rankId = FileUtil::GetProfilerFileId(FileUtil::SplicePath(filePath, "mindstudio_data.db"));
    return FileUtil::GetDbPath(FileUtil::SplicePath(filePath, "mindstudio_data.db"), rankId);
}
ProjectAnalyzeRegister<ProjectParserJson> pRegJson(ParserType::JSON);

} // Module
// Dic