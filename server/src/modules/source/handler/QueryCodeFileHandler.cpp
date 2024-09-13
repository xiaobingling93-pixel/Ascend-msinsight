/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "WsSessionManager.h"
#include "SourceProtocolRequest.h"
#include "SourceProtocolResponse.h"
#include "SourceFileParser.h"
#include "QueryCodeFileHandler.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;

void QueryCodeFileHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<SourceCodeFileRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<SourceCodeFileResponse> responsePtr = std::make_unique<SourceCodeFileResponse>();
    SourceCodeFileResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    if (auto [isVaild, errMsg] = request.params.Vaild(); isVaild == false) {
        ServerLog::Error("Parameter of command ", request.command, "is invaild, error:", errMsg);
        SetResponseResult(response, false, errMsg, ErrorCode::REQUEST_PARAMS_ERROR);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    const std::string &fileContent = SourceFileParser::Instance().GetSourceByName(request.params.sourceName);
    response.body.fileContent = fileContent;
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Source
} // Module
} // Dic