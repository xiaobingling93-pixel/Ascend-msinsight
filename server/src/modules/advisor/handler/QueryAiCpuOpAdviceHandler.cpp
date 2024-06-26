/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"
#include "AICpuOpAdvisor.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "QueryAiCpuOpAdviceHandler.h"

namespace Dic::Module::Advisor {
using namespace Dic::Server;
void QueryAiCpuOpAdviceHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<AICpuOperatorRequest &>(*requestPtr);
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session ai cpu op");
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<AICpuOperatorResponse> responsePtr = std::make_unique<AICpuOperatorResponse>();
    AICpuOperatorResponse &response = *responsePtr;
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
    if (!AICpuOpAdvisor::Process(request.params, response.body)) {
        ServerLog::Warn("Failed to Query AI CPU Operator Advice");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    session.OnResponse(std::move(responsePtr));
}
} // Dic::Module::Advisor