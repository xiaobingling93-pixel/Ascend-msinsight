/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"
#include "AclnnOpAdvisor.h"
#include "WsSessionManager.h"
#include "TraceTime.h"
#include "QueryAclnnOpAdvisorHandler.h"

namespace Dic::Module::Advisor {
using namespace Dic::Server;
bool QueryAclnnOpAdvisorHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<AclnnOperatorRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<AclnnOperatorResponse> responsePtr = std::make_unique<AclnnOperatorResponse>();
    AclnnOperatorResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    uint64_t minTimestamp = Timeline::TraceTime::Instance().GetStartTime();
    std::string error;
    request.params.Check(minTimestamp, error);
    if (!std::empty(error)) {
        ServerLog::Error(error);
        SetResponseResult(response, false, error);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    if (!AclnnOpAdvisor::Process(request.params, response.body)) {
        ServerLog::Error("Failed to Query Aclnn Operator Advice");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Dic::Module::Advisor