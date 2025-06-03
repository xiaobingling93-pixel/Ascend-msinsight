/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "TimelineRequestHandler.h"
#include "ModuleRequestHandler.h"
#include "FullDbParser.h"
#include "DataBaseManager.h"
#include "CommonDefs.h"
#include "ClusterFileParser.h"
#include "ClusterParseThreadPoolExecutor.h"
#include "ParserStatusManager.h"
#include "EventNotifyThreadPoolExecutor.h"
#include "TrackInfoManager.h"
#include "BaselineManager.h"
#include "TimeUtil.h"
#include "ProjectExplorerManager.h"
#include "ProjectParserJson.h"
#include "ProjectAnalyze.h"
#include "ProjectParserDb.h"

namespace Dic {
namespace Module {
using namespace Timeline;
void ProjectParserDb::Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request)
{
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr;
    ModuleRequestHandler::SetBaseResponse(request, response);
    Timeline::DataBaseManager::Instance().SetDataType(Timeline::DataType::DB);
    response.body.reset = IsNeedReset(request);
    if (response.body.reset) {
        ParserFactory::Reset();
    }
    // 待解析的数据文件单独处理
    std::vector<std::string> dataFiles = {};
    for (const auto &projectInfo : projectInfos) {
        for (const auto &item : projectInfo.subParseFileInfo) {
            dataFiles.push_back(item->parseFilePath);
            response.body.subParseFileInfo.push_back(item);
        }
    }
    response.body.projectFileTree = projectInfos[0].projectFileTree;
    auto hostInfoMap = GetReportFiles(dataFiles);
    SetHostInfo(hostInfoMap, response);
    SetParseCallBack();
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = MODULE_TIMELINE;
    ProjectTypeEnum curProjectTypeEnum = Global::ProjectExplorerManager::GetProjectType(projectInfos);
    response.body.isCluster = CheckIsOpenClusterTag(request.params.projectAction, curProjectTypeEnum,
                                                    projectInfos[0].projectName);
    response.body.isLeaks = DataBaseManager::Instance().GetFileTypeByRankId("") == FileType::LEAKS;
    bool isCluster = response.body.isCluster;
    bool isPending = response.body.isPending;
    // add response to response queue in session
    SendResponse(std::move(responsePtr), true);
    for (const auto &hostInfo : hostInfoMap) {
        for (const auto &ranks : hostInfo.second) {
            if (isPending) {
                ParserStatusManager::Instance().SetPendingStatus(ranks.second[0],
                    { ProjectTypeEnum::DB, { ranks.first } });
                continue;
            }
            FullDb::FullDbParser::Instance().Parse(ranks.second, ranks.first);
        }
    }
    // 执行集群数据解析
    ParseClusterInfo(projectInfos, isCluster, curProjectTypeEnum);
}

void ProjectParserDb::SetHostInfo(std::map<std::string, HostInfo> &hostInfoMap, ImportActionResponse &response)
{
    uint32_t rankSize = 0;
    for (auto &hostInfo : hostInfoMap) {
        if (!hostInfo.second.empty()) {
            // 如果rank列表为空，则Timeline页面不展示Host
            SetBaseActionOfResponse(response, "Host", hostInfo.first, "");
        }
        rankSize += hostInfo.second.size();
        for (auto &ranks : hostInfo.second) {
            for (auto &rank : ranks.second) {
                SetBaseActionOfResponse(response, rank, hostInfo.first, ranks.first);
                rank = hostInfo.first + rank;
            }
        }
    }
    bool isPendingParse = rankSize >= PENDIND_CRITICAL_VALUE;
    response.body.isPending = isPendingParse;
}

// LCOV_EXCL_BR_START
void ProjectParserDb::ClusterProcess(std::shared_ptr<ParseFileInfo> clusterInfo, bool isCluster,
                                     ProjectTypeEnum curProjectTypeEnum,
                                     std::map<std::string, std::vector<std::string>> &dataPathToDbMap,
                                     const std::string &projectName)
{
    std::string parseClusterResult = PARSE_RESULT_NONE;
    if (curProjectTypeEnum == ProjectTypeEnum::DB_CLUSTER) {
        auto clusterDatabase = DataBaseManager::Instance().CreateClusterDatabase(clusterInfo->parseFilePath,
                                                                                 DataType::DB);
        ClusterFileParser clusterFileParser(clusterInfo->parseFilePath, clusterDatabase,
                                            clusterInfo->parseFilePath + TimeUtil::Instance().NowStr());
        if (clusterFileParser.ParserClusterOfDb()) {
            ServerLog::Info("The cluster db file is parsed successfully.");
            dataPathToDbMap[clusterInfo->parseFilePath].push_back(clusterFileParser.GetClusterDbPath());
            parseClusterResult = PARSE_RESULT_OK;
        } else {
            ServerLog::Warn("Failed to parse cluster db files");
            parseClusterResult = PARSE_RESULT_FAIL;
        }
    }
    SaveDbPath(projectName, dataPathToDbMap);
    // send event
    ProjectParserBase::ParseClusterEndProcess(parseClusterResult, isCluster, clusterInfo->parseFilePath);
    // 全量db也必须发step2完成事件，否则通信耗时分析会卡住
    auto event = std::make_unique<ParseClusterStep2CompletedEvent>();
    event->moduleName = MODULE_TIMELINE;
    event->result = true;
    event->body.parseResult = std::move(parseClusterResult);
    event->body.clusterPath = clusterInfo->parseFilePath;
    SendEvent(std::move(event));
}
// LCOV_EXCL_BR_STOP

std::map<std::string, HostInfo> ProjectParserDb::GetReportFiles(const std::vector<std::string> &reportFiles)
{
    std::vector<std::string> dbFiles = {};
    for (const auto &file : reportFiles) {
        if (!FileUtil::IsFolder(file)) {
            dbFiles.push_back(file);
            if (std::regex_match(FileUtil::GetFileName(file), std::regex(leaksMemDbReg))) {
                DataBaseManager::Instance().SetFileType(FileType::LEAKS);
            }
            continue;
        }
        std::vector<std::string> dbFilesOneDir = GetDbFilesInDir(file);
        if (dbFilesOneDir.empty()) {
            continue;
        }
        if (RegexUtil::RegexSearch(dbFilesOneDir[0], leaksMemDbReg)) {
            std::copy(dbFilesOneDir.begin(), dbFilesOneDir.end(), std::back_inserter(dbFiles));
            DataBaseManager::Instance().SetFileType(FileType::LEAKS);
            continue;
        }
        std::copy(dbFilesOneDir.begin(), dbFilesOneDir.end(), std::back_inserter(dbFiles));
    }
    // 只解析找到的第一个report文件
    std::map<std::string, HostInfo> hostMap;
    for (const auto &file : dbFiles) {
        if (!Timeline::DataBaseManager::Instance().CreatConnectionPool(file, file)) {
            ServerLog::Error("Failed to create connection pool. ", dbFiles[0]);
        }
        auto db = Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId(file);
        if (db == nullptr) {
            ServerLog::Error("Failed to get connection.");
            continue;
        }
        auto database = std::dynamic_pointer_cast<FullDb::DbTraceDataBase, Timeline::VirtualTraceDatabase>(db);
        if (database == nullptr) {
            ServerLog::Error("Failed to convert virtual trace database to db trace database in getting report files.");
            continue;
        }
        auto host = database->QueryHostInfo();
        for (const auto &rank : database->QueryRankId()) {
            hostMap[host][file].push_back(rank);
            DataBaseManager::Instance().SetDbPathMapping(host + rank, file, host + "Host");
            TrackInfoManager::Instance().UpdateHost(host + rank, host);
            TrackInfoManager::Instance().UpdateDeviceMap(host + rank, database->QueryRankIdAndDeviceMap());
            TrackInfoManager::Instance().UpdateHostCardId(host + rank, file);
        }
    }
    return hostMap;
}

void ProjectParserDb::SetParseCallBack()
{
    std::function<void(const std::string, const std::string, bool, const std::string)> func =
        std::bind(ParseEndCallBack, std::placeholders::_1, std::placeholders::_2,
                  std::placeholders::_3,
                  std::placeholders::_4);
    FullDb::FullDbParser::Instance().SetParseEndCallBack(func);
}

void ProjectParserDb::SetBaseActionOfResponse(ImportActionResponse &response, const std::string &rankId,
    const std::string &host, const std::string &dbFile)
{
    Action action;
    action.cardName = rankId;
    action.rankId = host + rankId;
    action.host = host.length() >= 1 ? host.substr(0, host.length() - 1) : "";
    action.result = true;
    action.fileId = dbFile;
    if (!dbFile.empty()) {
        action.cardPath = "Directory: " + FileUtil::GetRankIdFromPath(dbFile);
        action.dataPathList.push_back(FileUtil::GetParentPath(dbFile));
    }
    response.body.result.emplace_back(action);
}

ProjectTypeEnum ProjectParserDb::GetProjectType(const std::string &dataPath)
{
    if (dataPath.empty()) {
        return ProjectTypeEnum::DB;
    }
    std::vector<std::string> leaksFiles = FileUtil::FindFilesWithFilter(dataPath, std::regex(leaksMemDbReg));
    if (!leaksFiles.empty()) {
        return ProjectTypeEnum::DB;
    }
    std::vector<std::string> frameworkFiles = FileUtil::FindFilesWithFilter(dataPath, std::regex(pytorchDBReg));
    if (frameworkFiles.empty()) {
        frameworkFiles = FileUtil::FindFilesWithFilter(dataPath, std::regex(mindsporeDBReg));
    }
    std::vector<std::string> msprofFiles = FileUtil::FindFilesWithFilter(dataPath, std::regex(msprofDBReg));
    std::vector<std::string> clusterPath = FileUtil::FindFilesWithFilter(dataPath, std::regex(clusterDBReg));
    int rankCount = frameworkFiles.size() + msprofFiles.size();
    // 如果rank的数据大于1个或导入的为cluster_analysis.db单文件，则判断需要进行集群分析
    bool isCluster = (rankCount > 1) || (rankCount == 0 && (clusterPath.size() > 0));
    return isCluster ? ProjectTypeEnum::DB_CLUSTER : ProjectTypeEnum::DB;
}

static bool IsSingleLeaksDbFile(const std::string &importFile)
{
    if (FileUtil::IsFolder(importFile)) {
        return false;
    }
    return std::regex_match(FileUtil::GetFileName(importFile), std::regex(leaksMemDbReg));
}

bool TryGetLeaksByImportFolderOrFile(const std::string &importFile, std::vector<std::string> &leaksFiles)
{
    leaksFiles = FileUtil::FindFilesWithFilter(importFile, std::regex(leaksMemDbReg));
    if (IsSingleLeaksDbFile(importFile) || !leaksFiles.empty()) {
        DataBaseManager::Instance().SetFileType(FileType::LEAKS);
        if (leaksFiles.empty()) {
            leaksFiles.push_back(importFile);
        }
        return true;
    }
    return false;
}

std::vector<std::string> ProjectParserDb::GetParseFileByImportFile(const std::string &importFile, std::string &error)
{
    std::vector<std::string> leaksFiles;
    if (TryGetLeaksByImportFolderOrFile(importFile, leaksFiles) && !leaksFiles.empty()) {
        return leaksFiles;
    }
    std::vector<std::string> frameworkFiles = FileUtil::FindFilesWithFilter(importFile, std::regex(pytorchDBReg));
    if (frameworkFiles.empty()) {
        frameworkFiles = FileUtil::FindFilesWithFilter(importFile, std::regex(mindsporeDBReg));
    }
    std::vector<std::string> msprofFiles = FileUtil::FindFilesWithFilter(importFile, std::regex(msprofDBReg));
    if (frameworkFiles.empty() && msprofFiles.empty()) {
        error = "No parsable db files found, Possible reasons:; 1.File not exist; "
                "2.The nesting depth of the imported sub-file exceeds 5; 3.The sub-file path length exceeds " +
                std::to_string(FileUtil::GetFilePathLengthLimit());
        ServerLog::Info(error);
        return { importFile };
    }
    std::vector<std::string> reportFiles = {};
    if (!frameworkFiles.empty()) {
        reportFiles = frameworkFiles;
        DataBaseManager::Instance().SetFileType(FileType::PYTORCH);
    } else if (!msprofFiles.empty()) {
        reportFiles = msprofFiles;
        DataBaseManager::Instance().SetFileType(FileType::MS_PROF);
    }
    std::vector<std::string> res;
    for (const auto &item : reportFiles) {
        res.push_back(FileUtil::GetParentPath(item));
    }
    return res;
}

// LCOV_EXCL_BR_START
void ProjectParserDb::ParserBaseline(const Global::ProjectExplorerInfo &projectInfo,
    Global::BaselineInfo &baselineInfo)
{
    if (projectInfo.fileInfoMap.empty()) {
        return;
    }
    if (baselineInfo.isCluster) {
        ProjectParserDb::ParseBaselineClusterInfo(projectInfo);
        return;
    }
    std::string parseFilePath = projectInfo.subParseFileInfo[0]->parseFilePath;
    std::vector<std::string> frameworkFiles = FileUtil::FindFilesWithFilter(parseFilePath, std::regex(pytorchDBReg));
    if (frameworkFiles.empty()) {
        frameworkFiles = FileUtil::FindFilesWithFilter(parseFilePath, std::regex(mindsporeDBReg));
    }
    std::vector<std::string> msprofFiles = FileUtil::FindFilesWithFilter(parseFilePath, std::regex(msprofDBReg));
    if (frameworkFiles.empty() && msprofFiles.empty()) {
        return;
    }
    std::string file;
    if (!frameworkFiles.empty()) {
        DataBaseManager::Instance().SetBaselineFileType(FileType::PYTORCH);
        file = frameworkFiles[0];
    } else {
        DataBaseManager::Instance().SetBaselineFileType(FileType::MS_PROF);
        file = msprofFiles[0];
    }
    Timeline::DataBaseManager::Instance().SetDataType(Timeline::DataType::DB);
    auto hostInfoMap = GetReportFiles({ parseFilePath });
    if (std::empty(hostInfoMap)) {
        Global::BaselineManager::Instance().SetBaselineInfo(baselineInfo);
        baselineInfo.errorMessage = "Db get host info failed!";
        return;
    }
    if (std::empty(hostInfoMap.begin()->second) || std::empty(hostInfoMap.begin()->second.begin()->second)) {
        Global::BaselineManager::Instance().SetBaselineInfo(baselineInfo);
        baselineInfo.errorMessage = "Db get rank info failed!";
        return;
    }
    std::string rankId = hostInfoMap.begin()->first + hostInfoMap.begin()->second.begin()->second[0];
    const std::string &cardId = rankId;
    baselineInfo.rankId = rankId;
    baselineInfo.cardName = "baseline" + cardId;
    baselineInfo.host = hostInfoMap.begin()->first;
    Global::BaselineManager::Instance().SetBaselineInfo(baselineInfo);
    if (!Timeline::DataBaseManager::Instance().CreatConnectionPool(rankId, file)) {
        ServerLog::Error("Failed to create baseline connection pool. ");
    }
    FullDb::FullDbParser::Instance().Parse(std::vector<std::string>{ rankId }, file);
}

void ProjectParserDb::ParseBaselineClusterInfo(const Global::ProjectExplorerInfo &projectInfos)
{
    std::vector<std::shared_ptr<ParseFileInfo>> clusterInfos = projectInfos.GetClusterInfos();
    if (clusterInfos.empty()) {
        auto cluster = std::make_shared<ParseFileInfo>();
        cluster->parseFilePath = projectInfos.fileName;
        cluster->type = ParseFileType::CLUSTER;
        cluster->clusterId = FileUtil::GetFileName(projectInfos.fileName);
        clusterInfos.emplace_back(cluster);
    }
    std::for_each(clusterInfos.begin(), clusterInfos.end(), [](const std::shared_ptr<ParseFileInfo>& item) {
        auto clusterDatabase = DataBaseManager::Instance().CreateClusterDatabase(item->parseFilePath, DataType::DB);
        ClusterFileParser clusterFileParser(item->parseFilePath, clusterDatabase,
                                            item->clusterId + TimeUtil::Instance().NowStr());
        if (!clusterFileParser.ParserClusterOfDb()) {
            ServerLog::Warn("Failed to parse cluster db files");
        }
    });
}

void ProjectParserDb::BuildProjectExploreInfo(ProjectExplorerInfo &info, const std::vector<std::string> &parsedFiles)
{
    ProjectParserBase::BuildProjectExploreInfo(info, parsedFiles);
    std::for_each(parsedFiles.begin(), parsedFiles.end(), [&info](const auto& parsedFile) {
        ProjectParserDb::BuildProjectFromParseFile(info, parsedFile);
    });
}

void ProjectParserDb::BuildProjectFromParseFile(Dic::Module::Global::ProjectExplorerInfo &info,
                                                const std::string &parsedFile)
{
    std::vector<std::string> parentFolders = GetParentFileList(info.fileName, parsedFile);
    // Db工程层次：project-cluster-host-rank
    auto parseFileInfoRank = std::make_shared<ParseFileInfo>();
    parseFileInfoRank->parseFilePath = parsedFile;
    parseFileInfoRank->type = ParseFileType::RANK;
    parseFileInfoRank->subId = parsedFile;
    parseFileInfoRank->curDirName = FileUtil::GetFileName(parsedFile);
    parseFileInfoRank->fileId = GetFileIdWithDb(parsedFile);
    // import single file
    if (FileUtil::IsRegularFile(parsedFile) || parseFileInfoRank->subId == info.fileName) {
        parseFileInfoRank->subId = FileUtil::GetFileName(parsedFile);
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

void ProjectParserDb::ParseClusterInfo(const std::vector<Global::ProjectExplorerInfo> &projectInfos, bool isCluster,
                                       ProjectTypeEnum projectType)
{
    auto clusterFilePath = Global::ProjectExplorerManager::GetClusterFilePath(projectInfos);
    if (clusterFilePath.empty()) {
        std::for_each(projectInfos.begin(), projectInfos.end(), [&clusterFilePath](const ProjectExplorerInfo &item) {
            auto cluster = std::make_shared<ParseFileInfo>();
            cluster->parseFilePath = item.fileName;
            cluster->type = ParseFileType::CLUSTER;
            cluster->clusterId = item.fileName;
            clusterFilePath.emplace_back(cluster);
        });
    }
    auto clusterParse = [isCluster, projectType, this, &projectInfos](const std::shared_ptr<ParseFileInfo> &cluster) {
        ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(
            ClusterProcess,
            cluster,
            isCluster,
            projectType,
            dataPathToDbMap,
            projectInfos[0].projectName);
    };
    std::for_each(clusterFilePath.begin(), clusterFilePath.end(), clusterParse);
    Timeline::EventNotifyThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ParsePostProcess, clusterFilePath);
}
// LCOV_EXCL_BR_STOP

std::string ProjectParserDb::GetFileIdWithDb(const std::string &filePath)
{
    if (!FileUtil::IsFolder(filePath)) {
        return filePath;
    }
    std::vector<std::string> dbFiles = GetDbFilesInDir(filePath);
    if (dbFiles.empty()) {
        return filePath;
    }
    return dbFiles[0]; // 以找到的第一个文件为准
}

std::vector<std::string> ProjectParserDb::GetDbFilesInDir(const std::string &filePath)
{
    // 静态初始化，避免重复调用时正则编译开销
    static std::vector<std::regex> dbRegex = {
        std::regex{leaksMemDbReg}, std::regex{pytorchDBReg},
        std::regex{mindsporeDBReg}, std::regex{msprofDBReg}};
    for (const auto &dbRegx: dbRegex) {
        std::vector<std::string> res = FileUtil::FindFilesWithFilter(filePath, dbRegx);
        if (!res.empty()) {
            return res;
        }
    }
    return {};
}

ProjectAnalyzeRegister<ProjectParserDb>  pRegDB(ParserType::DB);
} // Module
} // Dic