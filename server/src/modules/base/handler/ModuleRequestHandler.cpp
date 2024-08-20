/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "ModuleRequestHandler.h"
#include "pch.h"
#include "IdBuilder.h"
#include "WsSessionManager.h"

namespace Dic {
namespace Module {
using namespace Dic::Protocol;
using namespace Dic::Server;

const std::string ModuleRequestHandler::GetError()
{
    return error;
}

void ModuleRequestHandler::SetBaseResponse(const Request &request, Response &response)
{
    response.type = ProtocolMessage::Type::RESPONSE;
    response.id = static_cast<uint32_t>(IdBuilder::ResponseIdBuilder().Build());
    response.requestId = request.id;
    response.command = request.command;
    response.moduleName = request.moduleName;
    if (request.resultCallbackId.has_value()) {
        response.resultCallbackId = request.resultCallbackId;
    }
}

void ModuleRequestHandler::SetResponseResult(Response &response, bool result, const std::string &errorMsg,
                                             const ErrorCode &errorCode)
{
    response.result = result;
    if (result) {
        return;
    }
    if (errorMsg.empty()) {
        response.error = MakeError(ErrorCode::UNKNOW_ERROR, "");
    } else {
        ServerLog::Error(errorMsg);
        response.error = MakeError(errorCode, errorMsg);
    }
}

void ModuleRequestHandler::SendResponse(std::unique_ptr<Protocol::Response> responsePtr, bool result,
                                        const std::string &errorMsg, const ErrorCode &errorCode)
{
    WsSession &session = *WsSessionManager::Instance().GetSession();
    SetResponseResult(*responsePtr, result, errorMsg, errorCode);
    session.OnResponse(std::move(responsePtr));
}

bool ModuleRequestHandler::IsAsync()
{
    return async;
}
} // end of namespace Module
} // end of namespace Dic