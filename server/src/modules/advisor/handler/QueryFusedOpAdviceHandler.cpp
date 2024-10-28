/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"
#include "FusedOpAdvisor.h"
#include "WsSessionManager.h"
#include "QueryFusedOpAdviceHandler.h"

namespace Dic::Module::Advisor {
using namespace Dic::Server;
bool QueryFusedOpAdviceHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<OperatorFusionRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<OperatorFusionResponse> responsePtr = std::make_unique<OperatorFusionResponse>();
    OperatorFusionResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    std::string error;
    request.params.Check(error);
    if (!std::empty(error)) {
        ServerLog::Error(error);
        SetResponseResult(response, false, error);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    if (!FusedOpAdvisor::Process(request.params, response.body)) {
        ServerLog::Error("Failed to Query Fused Operator Advice for rank: ", request.params.rankId);
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Dic::Module::Advisor