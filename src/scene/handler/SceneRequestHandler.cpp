/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "IdBuilder.h"
#include "SceneRequestHandler.h"

namespace Dic {
namespace Scene {
using namespace Dic::Protocol;
using namespace Dic::Server;

const std::string SceneRequestHandler::GetError()
{
    return error;
}

void SceneRequestHandler::SetBaseResponse(const Request &request, Response &response) const
{
    response.type = ProtocolMessage::Type::RESPONSE;
    response.id = static_cast<uint32_t>(IdBuilder::ResponseIdBuilder().Build());
    response.requestId = request.id;
    response.command = request.command;
    response.scene = request.scene;
    response.token = request.token;
    if (request.resultCallbackId.has_value()) {
        response.resultCallbackId = request.resultCallbackId;
    }
}

void SceneRequestHandler::SetResponseResult(Response &response, bool result, const std::string &errorMsg,
                                            const ErrorCode &errorCode) const
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
} // end of namespace Scene
} // end of namespace Dic