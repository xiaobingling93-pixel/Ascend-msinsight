/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "WsSessionManager.h"
#include "TokenCreateHandler.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
void TokenCreateHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    TokenCreateRequest &request = dynamic_cast<TokenCreateRequest &>(*requestPtr.get());
    std::string token = request.token;
    ServerLog::Info("Token create start, token = ", StringUtil::AnonymousString(token));
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
            ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    // set session deadTime, do not use deadTime by now
    session.SetDeadTime(request.params.deadTime);
    std::unique_ptr<TokenCreateResponse> responsePtr = std::make_unique<TokenCreateResponse>();
    TokenCreateResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    response.body.createTime = session.GetCreateTime();
    std::string parentToken = request.params.parentToken.has_value() ? request.params.parentToken.value() : "";
    if (!parentToken.empty()) {
        response.body.parentToken = parentToken;
    }
    if (!session.BindToken(token, parentToken)) {
        error = "Failed to bind token, token = " + StringUtil::AnonymousString(token) +
            ", parentToken = " + StringUtil::AnonymousString(parentToken);
        SetResponseResult(response, false, error, ErrorCode::COMMAND_PARAMS_ERROR);
    } else {
        SetResponseResult(response, true);
    }
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // end of namespace Module
} // Dic