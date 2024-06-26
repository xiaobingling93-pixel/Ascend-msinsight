/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "QueryApiLineHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "SourceProtocolRequest.h"
#include "SourceProtocolResponse.h"
#include "SourceFileParser.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;

void QueryApiLineHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<SourceApiLineRequest &>(*requestPtr);
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Error("Failed to check session token , command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<SourceApiLineResponse> responsePtr = std::make_unique<SourceApiLineResponse>();
    SourceApiLineResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    const std::vector<SourceFileLine> &lines = SourceFileParser::Instance().GetApiLinesByCoreAndSource(
        request.params.coreName, request.params.sourceName);

    std::vector<SourceFileLineRes> lineResArray;
    for (auto line: lines) {
        SourceFileLineRes lineRes;
        lineRes.line = line.line;
        lineRes.cycle = line.cycles[0];
        lineRes.instructionExecuted = line.instructionsExecuted[0];
        std::vector<std::pair<std::string, std::string>> copiedVector(line.addressRange);
        lineRes.addressRange = copiedVector;

        lineResArray.emplace_back(lineRes);
    }
    response.body.lines = lineResArray;
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Source
} // Module
} // Dic