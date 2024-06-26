/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"
#include "AffinityAPIAdvisor.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "QueryAffinityAPIAdvice.h"

namespace Dic::Module::Advisor {
using namespace Dic::Server;
void QueryAffinityAPIAdvice::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<AffinityAPIRequest &>(*requestPtr);
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session affinity Api");
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<AffinityAPIResponse> responsePtr = std::make_unique<AffinityAPIResponse>();
    AffinityAPIResponse &response = *responsePtr;
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
    if (!AffinityAPIAdvisor::Process(request.params, response.body)) {
        ServerLog::Warn("Failed to Query Affinity API Advice for rank ", request.params.rankId);
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    session.OnResponse(std::move(responsePtr));
}
} // Dic::Module::Advisor