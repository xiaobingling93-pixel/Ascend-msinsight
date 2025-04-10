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
#include "ParserDb.h"

namespace Dic {
namespace Module {
using namespace Timeline;
ParserDb::ParserDb() {}

ParserDb::~ParserDb() {}

void ParserDb::Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request)
{
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr.get();
    ModuleRequestHandler::SetBaseResponse(request, response);
    Timeline::DataBaseManager::Instance().SetDataType(Timeline::DataType::DB);
    response.body.reset = IsNeedReset(request);
    if (response.body.reset) {
        ParserFactory::Reset();
    }
    std::vector<std::string> reportFiles = {};
    for (const auto &projectInfo : projectInfos) {
        for (const auto &item : projectInfo.parseFilePathInfos) {
            reportFiles.push_back(item.parseFilePath);
        }
    }
    auto hostInfoMap = GetReportFiles(reportFiles);
    SetHostInfo(hostInfoMap, response);
    response.body.subdirectoryList = reportFiles;
    SetParseCallBack();
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = MODULE_TIMELINE;
    ProjectTypeEnum curProjectTypeEnum = Global::ProjectExplorerManager::GetProjectType(projectInfos);
    response.body.isCluster = CheckIsOpenClusterTag(request.params.projectAction, curProjectTypeEnum,
                                                    projectInfos[0].projectName);
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
    std::string clusterFilePath = Global::ProjectExplorerManager::GetClusterFilePath(projectInfos);
    Timeline::ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ClusterProcess, clusterFilePath,
        isCluster, curProjectTypeEnum, dataPathToDbMap, projectInfos[0].projectName);
    Timeline::EventNotifyThreadPoolExecutor::Instance().GetThreadPool()->AddTask(SendAllParseSuccess);
}

void ParserDb::SetHostInfo(std::map<std::string, HostInfo> &hostInfoMap, ImportActionResponse &response)
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

void ParserDb::ClusterProcess(const std::string &selectedFolder, bool isCluster, ProjectTypeEnum curProjectTypeEnum,
    std::map<std::string, std::vector<std::string>> &dataPathToDbMap, const std::string &projectName)
{
    std::string parseClusterResult = PARSE_RESULT_NONE;
    if (curProjectTypeEnum == ProjectTypeEnum::DB_CLUSTER) {
        auto clusterDatabase = DataBaseManager::Instance().CreateClusterDatabase(COMPARE, DataType::DB);
        ClusterFileParser clusterFileParser(selectedFolder, clusterDatabase, COMPARE + TimeUtil::Instance().NowStr());
        if (clusterFileParser.ParserClusterOfDb()) {
            ServerLog::Info("The cluster db file is parsed successfully.");
            dataPathToDbMap[selectedFolder].push_back(clusterFileParser.GetClusterDbPath());
            parseClusterResult = PARSE_RESULT_OK;
        } else {
            ServerLog::Warn("Failed to parse cluster db files");
            parseClusterResult = PARSE_RESULT_FAIL;
        }
    }
    SaveDbPath(projectName, dataPathToDbMap);
    // send event
    ParserAlloc::ParseClusterEndProcess(parseClusterResult, isCluster);
    // 全量db也必须发step2完成事件，否则通信耗时分析会卡住
    auto event = std::make_unique<ParseClusterStep2CompletedEvent>();
    event->moduleName = MODULE_TIMELINE;
    event->result = true;
    event->body.parseResult = std::move(parseClusterResult);
    SendEvent(std::move(event));
}

std::map<std::string, HostInfo> ParserDb::GetReportFiles(const std::vector<std::string> &reportFiles)
{
    std::vector<std::string> dbFiles = {};
    for (const auto &file : reportFiles) {
        if (!FileUtil::IsFolder(file)) {
            dbFiles.push_back(file);
            continue;
        }
        std::vector<std::string> frameworkFiles = FileUtil::FindFilesWithFilter(file, std::regex(pytorchDBReg));
        if (frameworkFiles.empty()) {
            frameworkFiles = FileUtil::FindFilesWithFilter(file, std::regex(mindsporeDBReg));
        }
        std::vector<std::string> msprofFiles = FileUtil::FindFilesWithFilter(file, std::regex(msprofDBReg));
        dbFiles.insert(dbFiles.end(), frameworkFiles.begin(), frameworkFiles.end());
        dbFiles.insert(dbFiles.end(), msprofFiles.begin(), msprofFiles.end());
    }
    // 只解析找到的第一个report文件
    std::map<std::string, HostInfo> hostMap;
    for (const auto &file : dbFiles) {
        if (!Timeline::DataBaseManager::Instance().CreatConnectionPool(file, file)) {
            ServerLog::Error("Failed to create connection pool. ", dbFiles[0]);
        }
        auto db = Timeline::DataBaseManager::Instance().GetTraceDatabase(file);
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
        std::vector<std::string> rankIds = database->QueryRankId();
        std::unordered_map<std::string, std::string> rankAndDeviceMap = database->QueryRankIdAndDeviceMap();
        for (const auto &rank : rankIds) {
            hostMap[host][file].push_back(rank);
            DataBaseManager::Instance().SetDbPathMapping(host + rank, file, host + "Host");
            TrackInfoManager::Instance().UpdateHost(host + rank, host);
            TrackInfoManager::Instance().UpdateDeviceMap(host + rank, rankAndDeviceMap);
            TrackInfoManager::Instance().UpdateHostCardId(host + rank, file);
        }
    }
    return hostMap;
}

void ParserDb::SetParseCallBack()
{
    std::function<void(const std::string, bool, const std::string)> func =
        std::bind(ParseEndCallBack, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    FullDb::FullDbParser::Instance().SetParseEndCallBack(func);
}

void ParserDb::SetBaseActionOfResponse(ImportActionResponse &response, const std::string &rankId,
    const std::string &host, const std::string &dbFile)
{
    Action action;
    action.cardName = rankId;
    action.rankId = host + rankId;
    action.host = host.length() >= 1 ? host.substr(0, host.length() - 1) : "";
    action.result = true;
    if (!dbFile.empty()) {
        action.cardPath = "Directory: " + FileUtil::GetRankIdFromPath(dbFile);
        action.dataPathList.push_back(FileUtil::GetParentPath(dbFile));
    }
    response.body.result.emplace_back(action);
}

ProjectTypeEnum ParserDb::GetProjectType(const std::vector<std::string> &dataPath)
{
    if (dataPath.empty()) {
        return ProjectTypeEnum::DB;
    }
    std::vector<std::string> frameworkFiles = FileUtil::FindFilesWithFilter(dataPath[0], std::regex(pytorchDBReg));
    if (frameworkFiles.empty()) {
        frameworkFiles = FileUtil::FindFilesWithFilter(dataPath[0], std::regex(mindsporeDBReg));
    }
    std::vector<std::string> msprofFiles = FileUtil::FindFilesWithFilter(dataPath[0], std::regex(msprofDBReg));
    std::vector<std::string> clusterPath = FileUtil::FindFilesWithFilter(dataPath[0], std::regex(clusterDBReg));
    int rankCount = frameworkFiles.size() + msprofFiles.size();
    // 如果rank的数据大于1个或导入的为cluster_analysis.db单文件，则判断需要进行集群分析
    bool isCluster = (rankCount > 1) || (rankCount == 0 && (clusterPath.size() > 0));
    return isCluster ? ProjectTypeEnum::DB_CLUSTER : ProjectTypeEnum::DB;
}

std::vector<std::string> ParserDb::GetParseFileByImportFile(const std::string &importFile,
    ProjectTypeEnum projectTypeEnum, std::string &error)
{
    std::vector<std::string> frameworkFiles = FileUtil::FindFilesWithFilter(importFile, std::regex(pytorchDBReg));
    if (frameworkFiles.empty()) {
        frameworkFiles = FileUtil::FindFilesWithFilter(importFile, std::regex(mindsporeDBReg));
    }
    std::vector<std::string> msprofFiles = FileUtil::FindFilesWithFilter(importFile, std::regex(msprofDBReg));
    if (frameworkFiles.empty() && msprofFiles.empty()) {
        error = "Not find valid db dir!";
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

void ParserDb::ParserBaseline(const std::vector<Global::ProjectExplorerInfo> &projectInfos,
    Global::BaselineInfo &baselineInfo)
{
    if (projectInfos.empty() || projectInfos[0].parseFilePathInfos.empty()) {
        return;
    }
    if (baselineInfo.isCluster) {
        ParserDb::ParseClusterBaselineInfo(projectInfos);
        return;
    }
    std::string parseFilePath = projectInfos[0].parseFilePathInfos[0].parseFilePath;
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

void ParserDb::ParseClusterBaselineInfo(const std::vector<Global::ProjectExplorerInfo> &projectInfos)
{
    std::string clusterFilePath = Global::ProjectExplorerManager::GetClusterFilePath(projectInfos);
    auto clusterDatabase = DataBaseManager::Instance().CreateClusterDatabase(BASELINE, DataType::DB);
    ClusterFileParser clusterFileParser(clusterFilePath, clusterDatabase, BASELINE + TimeUtil::Instance().NowStr());
    if (!clusterFileParser.ParserClusterOfDb()) {
        ServerLog::Warn("Failed to parse cluster db files");
    }
}
} // Module
} // Dic