/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "ModuleRequestHandler.h"
#include "../utils/pch.h"
#include "../WsSessionManager.h"
#include "../utils/IdBuilder.h"

namespace Dic {
namespace Module {
using namespace Dic::Protocol;
using namespace Dic::Server;

bool ModuleRequestHandler::HandleRequestEntrance(std::unique_ptr<Request> requestPtr)
{
    unsigned int id = requestPtr->id;
    ServerLog::Info("Start handle request, module = ", moduleName, ", command = ", command, ", id = ", id);
    bool res = HandleRequest(std::move(requestPtr));
    ServerLog::Info("End handle request, module = ", moduleName, ", command = ", command, ", id = ", id);
    return res;
}

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
                                             const int errorCode)
{
    response.result = result;
    if (result) {
        return;
    }
    if (!errorMsg.empty()) {
        ServerLog::Error(errorMsg);
    }
    response.error = { .code = errorCode, .message = errorMsg };
}

bool ModuleRequestHandler::IsAsync()
{
    return async;
}
} // end of namespace Module
} // end of namespace Dic