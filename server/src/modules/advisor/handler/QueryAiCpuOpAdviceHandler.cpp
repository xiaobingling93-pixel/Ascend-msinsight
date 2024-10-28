/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"
#include "AICpuOpAdvisor.h"
#include "WsSessionManager.h"
#include "QueryAiCpuOpAdviceHandler.h"

namespace Dic::Module::Advisor {
using namespace Dic::Server;
bool QueryAiCpuOpAdviceHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<AICpuOperatorRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<AICpuOperatorResponse> responsePtr = std::make_unique<AICpuOperatorResponse>();
    AICpuOperatorResponse &response = *responsePtr;
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
    if (!AICpuOpAdvisor::Process(request.params, response.body)) {
        ServerLog::Error("Failed to Query AI CPU Operator Advice");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Dic::Module::Advisor