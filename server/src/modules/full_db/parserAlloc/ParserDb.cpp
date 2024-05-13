/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "ParserDb.h"
#include "FileUtil.h"
#include "TimelineRequestHandler.h"
#include "ModuleRequestHandler.h"
#include "FullDbParser.h"
#include "DataBaseManager.h"
#include "CommonDefs.h"
#include "ClusterFileParser.h"
#include "ClusterParseThreadPoolExecutor.h"
#include "ParserStatusManager.h"

namespace Dic {
namespace Module {
using namespace Timeline;
ParserDb::ParserDb() {
}

ParserDb::~ParserDb() {
}

void ParserDb::Parser(const std::string &path, ImportActionRequest &request)
{
    std::string token = request.token;
    Server::WsSession &session = *Server::WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr.get();
    ModuleRequestHandler::SetBaseResponse(request, response);

    Timeline::DataBaseManager::Instance().SetDataType(Timeline::DataType::FULL_DB);

    std::string selectedFolder = FileUtil::GetParentPath(path);
    std::map<std::string, std::string> devicePaths;
    FullDb::FullDbParser::FindDevicePaths(selectedFolder, devicePaths);
    auto rankList = GetReportFiles(path, response.body);
    if (!rankList.empty()) {
        // 如果rank列表为空，则Timeline页面不展示Host
        SetBaseActionOfResponse(response, "Host", devicePaths, "");
    }
    for (const auto &ranks: rankList) {
        for (const auto& rank: ranks.second) {
            SetBaseActionOfResponse(response, rank, devicePaths, ranks.first);
        }
    }

    SetParseCallBack(token);
    ModuleRequestHandler::SetResponseResult(response, true);
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = Protocol::ModuleType::TIMELINE;
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    for (const auto &ranks: rankList) {
        FullDb::FullDbParser::Instance().Parse(ranks.second, ranks.first, token);
    }
    std::vector<std::string> clusterPath = FileUtil::FindFilesWithFilter(path, std::regex(clusterDBReg));
    // 如果rank的数据大于1个或导入的为cluster_analysis.db单文件，则判断需要进行集群分析
    bool isCluster = (rankList.size() > 1) || (rankList.empty() && (clusterPath.size() > 0));
    // 执行集群数据解析
    Timeline::ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ClusterProcess, token, path,
                                                                                  isCluster);
}

void ParserDb::ClusterProcess(const std::string &token, const std::string &selectedFolder, bool isCluster)
{
    DataBaseManager::Instance().ClearClusterDb();
    std::string parseClusterResult = PARSE_RESULT_NONE;
    if (isCluster) {
        ServerLog::Info("ParseClusterFiles is success");
        parseClusterResult = PARSE_RESULT_OK;
        ClusterProcessAsyncStep(token, selectedFolder);
    } else {
        ServerLog::Warn("ParseClusterFiles is failed");
        parseClusterResult = PARSE_RESULT_FAIL;
    }
    // send event
    ParserAlloc::ParseClusterEndProcess(token, parseClusterResult);
}

void ParserDb::ClusterProcessAsyncStep(const std::string &token, const std::string &selectedFolder)
{
    std::string parseClusterResult;
    ClusterFileParser clusterFileParser;
    if (ParserStatusManager::Instance().GetClusterParserStatus() == ParserStatus::FINISH ||
        clusterFileParser.ParserClusterOfDb(selectedFolder)) {
        ServerLog::Info("ParseClusterDbFiles is success");
        ParserDb::curIsCluster = true;
        parseClusterResult = PARSE_RESULT_OK;
    } else {
        ServerLog::Warn("ParseClusterDbFiles is failed");
        ParserDb::curIsCluster = false;
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

std::map<std::string, std::vector<std::string>> ParserDb::GetReportFiles(const std::string &path,
    ImportActionResBody &body)
{
    std::vector<std::string> pytorchFiles = FileUtil::FindFilesWithFilter(path, std::regex(pytorchDBReg));
    std::vector<std::string> msprofFiles = FileUtil::FindFilesWithFilter(path, std::regex(msprofDBReg));
    CheckIfClusterAndReset(path, pytorchFiles.size(), body, true);
    if (pytorchFiles.empty() && msprofFiles.empty()) {
        Server::ServerLog::Warn("Failed to find db file.");
        return {};
    }
    std::vector<std::string> reportFiles = {};
    if (!pytorchFiles.empty()) {
        reportFiles = pytorchFiles;
        DataBaseManager::Instance().SetFileType(FileType::PYTORCH);
    } else if (!msprofFiles.empty()) {
        reportFiles = msprofFiles;
        DataBaseManager::Instance().SetFileType(FileType::MS_PROF);
    }
    // 只解析找到的第一个report文件
    std::map<std::string, std::vector<std::string>> rankListMap;
    for (const auto& file: reportFiles) {
        if (!Timeline::DataBaseManager::Instance().CreatConnectionPool(file, file)) {
            ServerLog::Error("Failed to create connection pool. ", reportFiles[0]);
        }
        auto db = Timeline::DataBaseManager::Instance().GetTraceDatabase(file);
        if (db == nullptr) {
            ServerLog::Error("Failed to get connection.");
            continue;
        }
        auto database = std::dynamic_pointer_cast<FullDb::DbTraceDataBase, Timeline::VirtualTraceDatabase>(db);
        if (database == nullptr) {
            ServerLog::Error("Failed to convert VirtualTraceDatabase to DbTraceDataBase in GetReportFiles.");
            continue;
        }
        std::vector<std::string> rankIds = database->QueryRankId();
        for (const auto& rank : rankIds) {
            rankListMap[file].push_back(rank);
            DataBaseManager::Instance().SetDbPathMapping(rank, file);
        }
    }
    return rankListMap;
}

void ParserDb::SetParseCallBack(std::string token)
{
    std::function<void(const std::string, bool, const std::string)> func =
            std::bind(ParseEndCallBack, token, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    FullDb::FullDbParser::Instance().SetParseEndCallBack(func);
}

void ParserDb::SetBaseActionOfResponse(ImportActionResponse &response, const std::string& rankId,
                                       std::map<std::string, std::string> devicePaths, const std::string& dbFile)
{
    Action action;
    action.cardName = rankId;
    action.rankId = rankId;
    action.result = true;
    if (devicePaths.find(rankId) != devicePaths.end()) {
        // 将文件所在路径的三级目录名称作为rank的tooltip信息
        action.cardPath = "Directory: " + devicePaths[rankId];
    } else if (!dbFile.empty()) {
        action.cardPath = "Directory: " + FileUtil::GetRankIdFromPath(dbFile);
    }
    response.body.result.emplace_back(action);
}
} // Module
} // Dic