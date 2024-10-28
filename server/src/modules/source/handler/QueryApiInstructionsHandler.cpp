/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "WsSessionManager.h"
#include "SourceProtocolRequest.h"
#include "SourceProtocolResponse.h"
#include "SourceFileParser.h"
#include "QueryApiInstructionsHandler.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;

bool QueryApiInstructionsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<SourceApiInstrRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<SourceApiInstrResponse> responsePtr = std::make_unique<SourceApiInstrResponse>();
    SourceApiInstrResponse &response = *responsePtr;
    SetBaseResponse(request, response);

    const std::string &instr = SourceFileParser::Instance().GetInstr();
    response.body.instructions = instr;
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // Source
} // Module
} // Dic