/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"
#include "AffinityOptimizerAdvisor.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "QueryAffinityOptimizerAdvice.h"

namespace Dic::Module::Advisor {
using namespace Dic::Server;
void QueryAffinityOptimizerAdvice::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<AffinityOptimizerRequest &>(*requestPtr);
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session affinity op");
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<AffinityOptimizerResponse> responsePtr = std::make_unique<AffinityOptimizerResponse>();
    AffinityOptimizerResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    std::string error;
    request.params.Check(error);
    if (!std::empty(error)) {
        ServerLog::Warn(error);
        SetResponseResult(response, false, error);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    if (!AffinityOptimizerAdvisor::Process(request.params, response.body)) {
        ServerLog::Warn("Failed to Query Affinity Optimizer Advice");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }

    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
}
} // Dic::Module::Advisor