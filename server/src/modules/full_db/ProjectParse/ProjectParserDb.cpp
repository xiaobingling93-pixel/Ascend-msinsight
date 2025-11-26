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
using namespace Dic::Server;
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
    auto hostInfoMap = GetReportFiles(projectInfos);
    SetHostInfo(hostInfoMap, response);
    SetParseCallBack();
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = MODULE_TIMELINE;
    ProjectTypeEnum curProjectTypeEnum = Global::ProjectExplorerManager::GetProjectType(projectInfos);
    response.body.isCluster = CheckIsOpenClusterTag(request.params.projectAction, curProjectTypeEnum,
                                                    projectInfos[0].projectName);
    response.body.isLeaks = DataBaseManager::Instance().GetFileType() == FileType::MEM_SCOPE;
    bool isCluster = response.body.isCluster;
    bool isPending = response.body.isPending;
    // add response to response queue in session
    SendResponse(std::move(responsePtr), true);
    std::for_each(projectInfos.begin(), projectInfos.end(), [](const auto& project) {
        if (!Global::ProjectExplorerManager::Instance().UpdateParseFileInfo(project.projectName,
                                                                            project.subParseFileInfo)) {
            ServerLog::Error("Failed to update project in parsing");
        }
    });
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

std::map<std::string, HostInfo> ProjectParserDb::GetReportFiles(const std::vector<ProjectExplorerInfo> &projectInfos,
                                                                std::optional<std::string> parseFilePathFilter)
{
    std::map<std::string, HostInfo> hostMap;
    for (const auto& project : projectInfos) {
        for (const auto& file : project.subParseFileInfo) {
            if (parseFilePathFilter.has_value() && file->parseFilePath != parseFilePathFilter.value()) {
                continue;
            }
            std::vector<std::string> dbFiles = GetDbFilesInDir(file->parseFilePath);
            std::for_each(dbFiles.begin(),
                          dbFiles.end(),
                          [&project, &hostMap, &file, this](const std::string &dbFile) {
                              GetReportFilesOneFile(project, hostMap, file, dbFile);
                          });
        }
    }
    return hostMap;
}

void ProjectParserDb::GetReportFilesOneFile(const Dic::Module::Global::ProjectExplorerInfo &project,
                                            std::map<std::string, HostInfo> &hostMap,
                                            std::shared_ptr<ParseFileInfo> parsefileInfo,
                                            const std::string &file)
{
    // 非RL数据不下发集群
    std::string clusterId = project.GetClusterInfos().empty() ? "" : parsefileInfo->clusterId;
    auto database = GetTraceDbConnect(file);
    if (!database) {
        return;
    }
    auto host = database->QueryHostInfo();
    auto rankList = database->QueryRankId();
    for (auto rank : rankList) {
        // 过滤-1的rankId, cann层感知不到rankId时用-1填充，暂时过滤，后续合并
        if (rank == "-1" && rankList.size() > 1) {
            ServerLog::Warn("Find rankId equals -1");
            continue;
        }
        if (parsefileInfo->type == DEVICE_CHIP) {
            if (rank == "-1") {
                rank = parsefileInfo->deviceId;
            } else if (parsefileInfo->deviceId != rank) {
                continue;
            }
        }
        parsefileInfo->host = host;
        parsefileInfo->rankId = host + rank;
        auto rankIdDeviceMap = database->QueryRankIdAndDeviceMap();
        auto deviceIdInMem = database->GetDeviceIdFromMemoryTable();
        SetRankDeviceMap(parsefileInfo, rankIdDeviceMap, deviceIdInMem, rank);
        std::string rankName = rank;
        hostMap[host][file].push_back(rank);
        DataBaseManager::Instance().SetDbPathMapping(host + rank, file, host + "Host");
        DataBaseManager::Instance().SetRankIdFileIdMapping(host + rank, file);
        TrackInfoManager::Instance().UpdateHost(host + rank, host);
        TrackInfoManager::Instance().UpdateDeviceMap(host + rank, rankIdDeviceMap);
        TrackInfoManager::Instance().UpdateDeviceToRankIdMap(host + parsefileInfo->deviceId, rank);
        TrackInfoManager::Instance().UpdateHostCardId(host + rank, file);
        TrackInfoManager::Instance().SetRankListByFileId(file,
                                                         {clusterId,
                                                          parsefileInfo->host,
                                                          parsefileInfo->rankId,
                                                          parsefileInfo->deviceId,
                                                          rankName});
        DataBaseManager::Instance().UpdateRankIdToDeviceId(file, host + rank, parsefileInfo->deviceId);
    }
    TrackInfoManager::Instance().AddRankToCluster(parsefileInfo->clusterId, parsefileInfo->rankId);
    TrackInfoManager::Instance().SetClusterByFileId(file, parsefileInfo->clusterId);
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
    action.cluster = TrackInfoManager::Instance().GetClusterByFileId(dbFile);
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
    std::vector<std::string> memScopeFiles = FileUtil::FindFilesWithFilter(dataPath, std::regex(memScopeDbReg));
    if (!memScopeFiles.empty()) {
        return ProjectTypeEnum::DB;
    }
    std::vector<std::string> frameworkFiles = FileUtil::FindFilesWithFilter(dataPath, std::regex(pytorchDBReg));
    if (frameworkFiles.empty()) {
        frameworkFiles = FileUtil::FindFilesWithFilter(dataPath, std::regex(mindsporeDBReg));
    }
    std::vector<std::string> msprofFiles = FileUtil::FindFilesWithFilter(dataPath, std::regex(msprofDBReg));
    std::vector<std::string> clusterPath = FileUtil::FindFilesWithFilter(dataPath, std::regex(clusterDBReg));
    uint64_t rankCount = frameworkFiles.size() + msprofFiles.size();
    // 如果rank的数据大于1个或导入的为cluster_analysis.db单文件，则判断需要进行集群分析
    bool isCluster = (rankCount > 1) || (rankCount == 0 && (clusterPath.size() > 0));
    return isCluster ? ProjectTypeEnum::DB_CLUSTER : ProjectTypeEnum::DB;
}

static bool IsSingleMemScopeDbFile(const std::string &importFile)
{
    if (FileUtil::IsFolder(importFile)) {
        return false;
    }
    return std::regex_match(FileUtil::GetFileName(importFile), std::regex(memScopeDbReg));
}

bool TryGetMemScopeFilesByImportFolderOrFile(const std::string &importFile, std::vector<std::string> &memScopeFiles)
{
    memScopeFiles = FileUtil::FindFilesWithFilter(importFile, std::regex(memScopeDbReg));
    if (IsSingleMemScopeDbFile(importFile) || !memScopeFiles.empty()) {
        DataBaseManager::Instance().SetFileType(FileType::MEM_SCOPE);
        if (memScopeFiles.empty()) {
            memScopeFiles.push_back(importFile);
        }
        return true;
    }
    return false;
}

std::vector<std::string> ProjectParserDb::GetParseFileByImportFile(const std::string &importFile, std::string &error)
{
    std::vector<std::string> memScopeFiles;
    if (TryGetMemScopeFilesByImportFolderOrFile(importFile, memScopeFiles) && !memScopeFiles.empty()) {
        return memScopeFiles;
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
        ProjectParserDb::ParseBaselineClusterInfo(projectInfo, baselineInfo);
        return;
    }
    std::string parseFilePath = baselineInfo.parsedFilePath;
    std::string file = GetBaselineDbFile(parseFilePath);
    if (file.empty()) {
        return;
    }
    Timeline::DataBaseManager::Instance().SetDataType(Timeline::DataType::DB);
    bool isParsed = Timeline::DataBaseManager::Instance().IsContainDatabasePath(file);
    auto hostInfoMap = GetReportFiles({projectInfo}, parseFilePath);
    if (std::empty(hostInfoMap)) {
        Global::BaselineManager::Instance().SetBaselineInfo(baselineInfo);
        baselineInfo.errorMessage = "Db get host info failed!";
        return;
    }
    FilterHostMap(hostInfoMap, parseFilePath);
    if (std::empty(hostInfoMap.begin()->second) || std::empty(hostInfoMap.begin()->second.begin()->second)) {
        Global::BaselineManager::Instance().SetBaselineInfo(baselineInfo);
        baselineInfo.errorMessage = "Db get rank info failed!";
        return;
    }
    std::string rankId = hostInfoMap.begin()->first + hostInfoMap.begin()->second.begin()->second[0];
    const std::string &cardId = rankId;
    baselineInfo.rankId = rankId;
    baselineInfo.cardName = "Baseline_" + cardId;
    baselineInfo.host = hostInfoMap.begin()->first;
    baselineInfo.fileId = file;
    Global::BaselineManager::Instance().SetBaselineInfo(baselineInfo);
    if (!Timeline::DataBaseManager::Instance().CreateTraceConnectionPool(baselineInfo.rankId, file)) {
        ServerLog::Error("Failed to create baseline connection pool. ");
    }
    if (isParsed) {
        ServerLog::Info("Baseline has parsed.");
        return;
    }
    FullDb::FullDbParser::Instance().Parse(std::vector<std::string>{ baselineInfo.rankId }, file);
}

void ProjectParserDb::FilterHostMap(std::map<std::string, HostInfo> &hostInfoMap, const std::string &filePath)
{
    // filter files
    for (auto hostIt = hostInfoMap.begin(); hostIt != hostInfoMap.end();) {
        auto &ranksInfo = hostIt->second;
        for (auto it = ranksInfo.begin(); it != ranksInfo.end();) {
            if (it->first.find(filePath) == std::string::npos) {
                it = ranksInfo.erase(it);
            } else {
                it ++;
            }
        }
        if (hostIt->second.empty()) {
            hostIt = hostInfoMap.erase(hostIt);
        } else {
            hostIt++;
        }
    }
}

void ProjectParserDb::ParseBaselineClusterInfo(const Global::ProjectExplorerInfo &projectInfos,
                                               BaselineInfo &baselineInfo)
{
    std::vector<std::shared_ptr<ParseFileInfo>> clusterInfos = projectInfos.GetClusterInfos();
    if (clusterInfos.empty()) {
        auto cluster = std::make_shared<ParseFileInfo>();
        cluster->parseFilePath = projectInfos.fileName;
        cluster->type = ParseFileType::CLUSTER;
        cluster->clusterId = FileUtil::GetFileName(projectInfos.fileName);
        clusterInfos.emplace_back(cluster);
    }
    std::for_each(clusterInfos.begin(), clusterInfos.end(), [&baselineInfo](const std::shared_ptr<ParseFileInfo>& item) {
        auto clusterDatabase = DataBaseManager::Instance().CreateClusterDatabase(item->parseFilePath, DataType::DB);
        ClusterFileParser clusterFileParser(item->parseFilePath, clusterDatabase,
                                            item->clusterId + TimeUtil::Instance().NowStr());
        if (!clusterFileParser.ParserClusterOfDb()) {
            ServerLog::Warn("Failed to parse cluster db files");
        }
        baselineInfo.fileId = clusterFileParser.GetClusterDbPath();
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
    if (FileUtil::IsRegularFile(parsedFile) && parseFileInfoRank->subId == info.fileName) {
        parseFileInfoRank->subId = FileUtil::GetFileName(parsedFile);
        info.AddSubParseFileInfo(info.fileName, ParseFileType::PROJECT, parseFileInfoRank);
        return;
    }
    // 设置cluster信息
    std::string cluster;
    std::string clusterPrefix;
    constexpr uint64_t clusterFolderCount = 2;
    if (parentFolders.size() >= clusterFolderCount && !IsMindFormsRankData(parentFolders)) {
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

    if (!clusterPrefix.empty()) {
        parseFileInfoRank->clusterId = FileUtil::GetFileName(cluster);
    } else {
        parseFileInfoRank->clusterId = FileUtil::GetFileName(info.fileName);
    }
    AddRankDeviceParseFileInfo(info, parseFileInfoRank);
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
            cluster->subParseFile = item.subParseFileInfo;
            clusterFilePath.emplace_back(cluster);
        });
    }
    auto clusterParse = [isCluster, projectType, this, &projectInfos](const std::shared_ptr<ParseFileInfo> &cluster) {
        ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(
            ClusterProcess,
            TraceIdManager::GetTraceId(),
            cluster,
            isCluster,
            projectType,
            dataPathToDbMap,
            projectInfos[0].projectName);
    };
    std::for_each(clusterFilePath.begin(), clusterFilePath.end(), clusterParse);
    Timeline::EventNotifyThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ParsePostProcess,
        TraceIdManager::GetTraceId(), clusterFilePath);
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
    std::vector<std::string> dbFiles;
    if (!FileUtil::IsFolder(filePath)) {
        dbFiles.emplace_back(filePath);
        if (std::regex_match(FileUtil::GetFileName(filePath), std::regex(memScopeDbReg))) {
            DataBaseManager::Instance().SetFileType(FileType::MEM_SCOPE);
        }
        return dbFiles;
    }
    // 静态初始化，避免重复调用时正则编译开销
    static std::vector<std::regex> dbRegex = {
        std::regex{memScopeDbReg}, std::regex{pytorchDBReg},
        std::regex{mindsporeDBReg}, std::regex{msprofDBReg}};
    for (const auto &dbRegx: dbRegex) {
        std::vector<std::string> res = FileUtil::FindFilesWithFilter(filePath, dbRegx);
        if (!res.empty()) {
            return res;
        }
    }
    return {};
}
std::shared_ptr<FullDb::DbTraceDataBase> ProjectParserDb::GetTraceDbConnect(const std::string &fileId)
{
    if (!Timeline::DataBaseManager::Instance().CreateTraceConnectionPool(fileId, fileId)) {
        ServerLog::Error("[ProjectParser] Failed to create connection pool. ", fileId);
    }
    auto db = Timeline::DataBaseManager::Instance().GetTraceDatabaseByFileId(fileId);
    if (db == nullptr) {
        ServerLog::Error("[ProjectParser] Failed to get connection.", fileId);
        return nullptr;
    }
    auto database = std::dynamic_pointer_cast<FullDb::DbTraceDataBase, Timeline::VirtualTraceDatabase>(db);
    if (database == nullptr) {
        ServerLog::Error(
            "[ProjectParser] Failed to convert virtual trace database to db trace database in getting report files.");
        return nullptr;
    }
    return database;
}
std::string ProjectParserDb::GetBaselineDbFile(const std::string &path)
{
    std::vector<std::string> frameworkFiles = FileUtil::FindFilesWithFilter(path, std::regex(pytorchDBReg));
    if (frameworkFiles.empty()) {
        frameworkFiles = FileUtil::FindFilesWithFilter(path, std::regex(mindsporeDBReg));
    }
    std::vector<std::string> msprofFiles = FileUtil::FindFilesWithFilter(path, std::regex(msprofDBReg));
    if (frameworkFiles.empty() && msprofFiles.empty()) {
        return "";
    }
    std::string file;
    if (!frameworkFiles.empty()) {
        DataBaseManager::Instance().SetBaselineFileType(FileType::PYTORCH);
        file = frameworkFiles[0];
    } else {
        DataBaseManager::Instance().SetBaselineFileType(FileType::MS_PROF);
        file = msprofFiles[0];
    }
    return file;
}
void ProjectParserDb::SetRankDeviceMap(std::shared_ptr<ParseFileInfo> parseFileInfo,
                                       std::unordered_map<std::string, std::string> &rankDeviceMap,
                                       const std::string &deviceIdInMem,
                                       const std::string &rank)
{
    if (parseFileInfo->type == DEVICE_CHIP) {
        rankDeviceMap.clear();
        rankDeviceMap[parseFileInfo->rankId] = parseFileInfo->deviceId;
        return;
    }
    if (rankDeviceMap.find(rank) != rankDeviceMap.end()) {
        parseFileInfo->deviceId = StringUtil::StrNumMax(rankDeviceMap[rank], parseFileInfo->deviceId);
    } else if (!deviceIdInMem.empty() && parseFileInfo->type != DEVICE_CHIP) {
        parseFileInfo->deviceId = StringUtil::StrNumMax(deviceIdInMem, parseFileInfo->deviceId);
    }
}

ProjectAnalyzeRegister<ProjectParserDb>  pRegDB(ParserType::DB);
} // Module
} // Dic