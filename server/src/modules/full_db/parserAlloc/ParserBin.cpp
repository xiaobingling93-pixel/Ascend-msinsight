/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "TimelineRequestHandler.h"
#include "WsSession.h"
#include "WsSessionManager.h"
#include "ModuleRequestHandler.h"
#include "CommonDefs.h"
#include "DataBaseManager.h"
#include "SourceFileParser.h"
#include "TraceTime.h"
#include "BaselineManager.h"
#include "ParserBin.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
ParserBin::ParserBin() {}

ParserBin::~ParserBin() {}

void ParserBin::Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request)
{
    Server::WsSession &session = *Server::WsSessionManager::Instance().GetSession();
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr.get();
    ModuleRequestHandler::SetBaseResponse(request, response);
    if (std::empty(projectInfos)) {
        SendParseFailEvent("", "Project explorer info is not existed.");
        // 这里需要返回一个true应答,否则前端会陷入不停loading中
        responsePtr->result = true;
        session.OnResponse(std::move(responsePtr));
        return;
    }
    Timeline::DataBaseManager::Instance().SetDataType(Timeline::DataType::TEXT);

    ModuleRequestHandler::SetResponseResult(response, true);
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = MODULE_TIMELINE;
    response.body.reset = true;
    response.body.subdirectoryList.push_back(projectInfos[0].fileName);

    std::string selectedFolder = projectInfos[0].fileName;

    // compute
    if (FileUtil::CheckFilePathLength(selectedFolder) &&
        Source::SourceFileParser::Instance().CheckOperatorBinary(selectedFolder)) {
        ServerLog::Info("Import file is binary.Start parse source binary file.");
        Source::SourceFileParser::Instance().SetFilePath(selectedFolder);
        HandleCompute(response, selectedFolder);
        TraceTime::Instance().SetIsSimulation(true);
        SaveDbPath(projectInfos[0].projectName, dataPathToDbMap);
        SetParseCallBack(Source::SourceFileParser::Instance());
        session.OnResponse(std::move(responsePtr));
    } else {
        SendParseFailEvent("", "Import file is invalid,path :" + selectedFolder);
        // 这里需要返回一个true应答,否则前端会陷入不停loading中
        responsePtr->result = true;
        session.OnResponse(std::move(responsePtr));
    }
}

void ParserBin::HandleCompute(ImportActionResponse &response, const std::string &selectedFolder)
{
    ServerLog::Info("Start parser source binary.");
    Source::SourceFileParser &sourceFileParser = Source::SourceFileParser::Instance();
    sourceFileParser.Reset();
    std::vector<std::string> empty;
    auto files = GetSimulationTraceFiles(selectedFolder, response.body);
    response.body.isSimulation = true;
    std::map<std::string, std::vector<std::string>> rankListMap = FileUtil::SplitToRankList(files);
    sourceFileParser.Parse(empty, files.front().second, selectedFolder);
    sourceFileParser.ConvertToData();
    for (const auto &rankEntry : rankListMap) {
        if (rankEntry.second.empty()) {
            continue;
        }
        std::string cardPath = FileUtil::GetRankIdFromPath(rankEntry.second[0]);
        SetBaseActionOfResponse(response, rankEntry.first, cardPath, std::vector<std::string>{});
    }
    ModuleRequestHandler::SetResponseResult(response, true);
    response.body.isBinary = true;
    response.body.coreList = sourceFileParser.GetCoreList();
    response.body.sourceList = sourceFileParser.GetSourceList();
}

std::vector<std::pair<std::string, std::string>> ParserBin::GetSimulationTraceFiles(const std::string &selectFilePath,
    ImportActionResBody &body)
{
    body.isCluster = false;
    std::vector<std::pair<std::string, std::string>> files;
    std::string fileId = GetFileId(selectFilePath, selectFilePath);
    if (fileId.empty()) {
        ServerLog::Error("File id is empty");
        return files;
    }
    files.emplace_back(selectFilePath, fileId);
    return files;
}

void ParserBin::SetParseCallBack(FileParser &fileParser)
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

ProjectTypeEnum ParserBin::GetProjectType(const std::vector<std::string> &dataPath)
{
    return ProjectTypeEnum::BIN;
}

void ParserBin::ParserBaseline(const std::vector<Global::ProjectExplorerInfo> &projectInfos,
                               Global::BaselineInfo &baselineInfo)
{
    if (projectInfos.empty() || projectInfos[0].parseFilePathInfos.empty()) {
        return;
    }
    std::string filePath = projectInfos[0].parseFilePathInfos[0].parseFilePath;
    std::string fileId = GetFileId(filePath, filePath);
    std::string dbPath = FileUtil::GetDbPath(filePath, fileId);
    baselineInfo.rankId = fileId;
    baselineInfo.cardName = "baseline" + fileId;
    Global::BaselineManager::Instance().SetBaselineInfo(baselineInfo);
    if (!DataBaseManager::Instance().CreatConnectionPool(fileId, dbPath)) {
        ServerLog::Error("Fail to create connection pool, fileId:", fileId, ", path:", dbPath, '.');
        return;
    }
    Source::SourceFileParser &sourceFileParser = Source::SourceFileParser::Instance();
    // 如果文件已经被解析则直接返回
    if (sourceFileParser.IsBaselineParsed(filePath)) {
        sourceFileParser.SynchronizeBaselineInfo();
        return;
    }
    // 只需要对比timeline和details页面内容，因此不需要对source相关的内容做处理
    if (sourceFileParser.CheckOperatorBinary(filePath)) {
        sourceFileParser.SetBaselineFilePath(filePath);
        sourceFileParser.Parse(std::vector<std::string>(), fileId, filePath);
    }
}
} // Module
} // Dic