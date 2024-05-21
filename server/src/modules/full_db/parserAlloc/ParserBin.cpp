/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "ParserBin.h"
#include "FileUtil.h"
#include "TimelineRequestHandler.h"
#include "WsSession.h"
#include "WsSessionManager.h"
#include "ModuleRequestHandler.h"
#include "TraceFileParser.h"
#include "FullDbParser.h"
#include "CommonDefs.h"
#include "DataBaseManager.h"
#include "SourceFileParser.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
ParserBin::ParserBin() {}

ParserBin::~ParserBin() {}

void ParserBin::Parser(const std::string &path, ImportActionRequest &request)
{
    std::string token = request.token;

    Server::WsSession &session = *Server::WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr.get();
    ModuleRequestHandler::SetBaseResponse(request, response);

    Timeline::DataBaseManager::Instance().SetDataType(Timeline::DataType::JSON);

    ModuleRequestHandler::SetResponseResult(response, true);
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = Protocol::ModuleType::TIMELINE;
    response.body.reset = true;

    std::string selectedFolder = request.params.path[0];

    // compute
    if (FileUtil::CheckFilePathLength(selectedFolder) &&
        Source::SourceFileParser::Instance().CheckOperatorBinary(selectedFolder)) {
        ServerLog::Info("Import file is binary.Start parse source binary file.");
        HandleCompute(response, selectedFolder);
        SetParseCallBack(token, Source::SourceFileParser::Instance());
        session.OnResponse(std::move(responsePtr));
    } else {
        SendParseFailEvent(token, "", "Import file is invalid,path :" + selectedFolder);
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
        SetBaseActionOfResponse(response, rankEntry);
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
    std::string fileId = GetFileId(selectFilePath);
    if (fileId.empty()) {
        ServerLog::Error("File id is empty. file:", selectFilePath);
        return files;
    }
    files.emplace_back(selectFilePath, fileId);
    return files;
}

void ParserBin::SetParseCallBack(const std::string &token, FileParser &fileParser)
{
    std::function<void(const std::string, bool, const std::string)> func =
        std::bind(ParseEndCallBack, token, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    fileParser.SetParseEndCallBack(func);
}
} // Module
} // Dic