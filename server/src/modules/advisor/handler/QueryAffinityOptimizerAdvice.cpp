/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"
#include "AffinityOptimizerAdvisor.h"
#include "WsSessionManager.h"
#include "QueryAffinityOptimizerAdvice.h"

namespace Dic::Module::Advisor {
using namespace Dic::Server;
bool QueryAffinityOptimizerAdvice::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<AffinityOptimizerRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<AffinityOptimizerResponse> responsePtr = std::make_unique<AffinityOptimizerResponse>();
    AffinityOptimizerResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    std::string error;
    request.params.Check(error);
    if (!std::empty(error)) {
        ServerLog::Error(error);
        SetResponseResult(response, false, error);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    if (!AffinityOptimizerAdvisor::Process(request.params, response.body)) {
        ServerLog::Error("Failed to Query Affinity Optimizer Advice");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return false;
    }

    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Dic::Module::Advisor