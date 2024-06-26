/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "QueryCodeFileHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "SourceProtocolRequest.h"
#include "SourceProtocolResponse.h"
#include "SourceFileParser.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;

void QueryCodeFileHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<SourceCodeFileRequest &>(*requestPtr);
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Error("Failed to check session token , command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<SourceCodeFileResponse> responsePtr = std::make_unique<SourceCodeFileResponse>();
    SourceCodeFileResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    const std::string &fileContent = SourceFileParser::Instance().GetSourceByName(request.params.sourceName);
    response.body.fileContent = fileContent;
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Source
} // Module
} // Dic