/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "pch.h"
#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"
#include "WsSessionManager.h"
#include "TraceTime.h"
#include "OperatorDispatchAdvisor.h"
#include "QueryOperatorDispatchHandler.h"

namespace Dic::Module::Advisor {
using namespace Dic::Server;
bool QueryOperatorDispatchHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<OperatorDispatchRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<OperatorDispatchResponse> responsePtr = std::make_unique<OperatorDispatchResponse>();
    OperatorDispatchResponse &response = *responsePtr;
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
    if (!OperatorDispatchAdvisor::Process(request.params, response.body)) {
        ServerLog::Error("Failed to Query Operator Dispatch Advice for rank: ", request.params.rankId);
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Dic::Module::Advisor
