/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "ParserDb.h"
#include "FileUtil.h"
#include "TimelineRequestHandler.h"
#include "WsSession.h"
#include "WsSessionManager.h"
#include "ModuleRequestHandler.h"
#include "FullDbParser.h"
#include "DataBaseManager.h"
#include "CommonDefs.h"
#include "TraceTime.h"
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
    for (const auto &rank: rankList) {
        SetBaseActionOfResponse(response, rank, devicePaths);
    }

    SetParseCallBack(token);
    ModuleRequestHandler::SetResponseResult(response, true);
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = Protocol::ModuleType::TIMELINE;
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    FullDb::FullDbParser::Instance().Parse({path}, "FullDb", token);
    // 执行集群数据解析
    Timeline::ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ClusterProcess, token, path,
        (rankList.size() > 1));
}

void ParserDb::ClusterProcess(const std::string &token, const std::string &selectedFolder, bool isCluster)
{
    std::string parseClusterResult = PARSE_RESULT_NONE;
    if (isCluster && FullDb::FullDbParser::Instance().InitCluster(selectedFolder, token)) {
        ServerLog::Info("ParseClusterFiles is success");
        parseClusterResult = PARSE_RESULT_OK;
        ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ClusterProcessAsyncStep,
                                                                            token, selectedFolder);
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
        parseClusterResult = PARSE_RESULT_OK;
    } else {
        ServerLog::Warn("ParseClusterDbFiles is failed");
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

std::vector<std::string> ParserDb::GetReportFiles(const std::string &path, ImportActionResBody &body)
{
    std::vector<std::string> reportFiles = FileUtil::FindFilesByRegex(path, std::regex(DBReg));
    CheckIfClusterAndReset(path, reportFiles.size(), body, true);
    if (reportFiles.empty()) {
        Server::ServerLog::Warn("Failed to find db file.");
        return {};
    }
    // 只解析找到的第一个report文件
    if (!Timeline::DataBaseManager::Instance().CreatConnectionPool("FullDb", reportFiles[0])) {
        ServerLog::Error("Failed to create connection pool. ", reportFiles[0]);
    }
    auto database = std::dynamic_pointer_cast<FullDb::DbTraceDataBase, Timeline::VirtualTraceDatabase>(
        Timeline::DataBaseManager::Instance().GetTraceDatabase("FullDb"));
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection.");
    }

    std::vector<std::string> rankIds = database->QueryRankId();
    std::map<std::string, std::vector<std::string>> rankListMap;
    for (auto rank : rankIds) {
        rankListMap[rank].push_back(path);
    }
    return rankIds;
}

void ParserDb::SetParseCallBack(std::string token)
{
    std::function<void(const std::string, bool, const std::string)> func =
            std::bind(ParseEndCallBack, token, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    FullDb::FullDbParser::Instance().SetParseEndCallBack(func);
}

void ParserDb::SetBaseActionOfResponse(ImportActionResponse &response, std::string rankId,
                                       std::map<std::string, std::string> devicePaths)
{
    Action action;
    action.cardName = rankId;
    action.rankId = rankId;
    action.result = true;
    // 将文件所在路径的三级目录名称作为rank的tooltip信息
    action.cardPath = "Directory: " + devicePaths[rankId];
    response.body.result.emplace_back(action);
}
} // Module
} // Dic