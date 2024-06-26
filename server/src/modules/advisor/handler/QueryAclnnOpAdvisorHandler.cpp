/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"
#include "AclnnOpAdvisor.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "QueryAclnnOpAdvisorHandler.h"

namespace Dic::Module::Advisor {
using namespace Dic::Server;
void QueryAclnnOpAdvisorHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<AclnnOperatorRequest &>(*requestPtr);
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session aclnn op");
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<AclnnOperatorResponse> responsePtr = std::make_unique<AclnnOperatorResponse>();
    AclnnOperatorResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    std::string error;
    request.params.Check(error);
    if (!std::empty(error)) {
        ServerLog::Warn(error);
        SetResponseResult(response, false, error);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    if (!AclnnOpAdvisor::Process(request.params, response.body)) {
        ServerLog::Warn("Failed to Query Aclnn Operator Advice");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    session.OnResponse(std::move(responsePtr));
}
} // Dic::Module::Advisor