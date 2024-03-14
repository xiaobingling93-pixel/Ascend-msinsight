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
    SetBaseActionOfResponse(response, "Host", devicePaths);
    for (const auto &ranks: rankList) {
        for (auto rank: ranks.second) {
            SetBaseActionOfResponse(response, rank, devicePaths);
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
    // 执行集群数据解析
    Timeline::ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->AddTask(ClusterProcess, token, path,
        (rankList.size() > 1));
}

void ParserDb::ClusterProcess(const std::string &token, const std::string &selectedFolder, bool isCluster)
{
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

std::map<std::string, std::vector<std::string>> ParserDb::GetReportFiles(const std::string &path,
    ImportActionResBody &body)
{
    std::vector<std::string> pytorchFiles = FileUtil::FindFilesByRegex(path, std::regex(pytorchDBReg));
    std::vector<std::string> msprofFiles = FileUtil::FindFilesByRegex(path, std::regex(msprofDBReg));
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
    for (auto file: reportFiles) {
        if (!Timeline::DataBaseManager::Instance().CreatConnectionPool(file, file)) {
            ServerLog::Error("Failed to create connection pool. ", reportFiles[0]);
        }
        auto database = std::dynamic_pointer_cast<FullDb::DbTraceDataBase, Timeline::VirtualTraceDatabase>(
            Timeline::DataBaseManager::Instance().GetTraceDatabase(file));
        if (database == nullptr) {
            ServerLog::Error("Failed to get connection.");
            continue;
        }
        std::vector<std::string> rankIds = database->QueryRankId();
        for (auto rank : rankIds) {
            rankListMap[file].push_back(rank);
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