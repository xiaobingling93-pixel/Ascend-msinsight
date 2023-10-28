/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "StepHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic::Server;
void StepHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    PipelineStepRequest &request = dynamic_cast<PipelineStepRequest &>(*requestPtr.get());
    if (!WsSessionManager::Instance().CheckSession(request.token)) {
        ServerLog::Error("Failed to check session token , command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(request.token);
    std::unique_ptr<PipelineStepResponse> responsePtr = std::make_unique<PipelineStepResponse>();
    PipelineStepResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase();
    if (!database->GetStepIdList(response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get step response data.");
    }
    session.OnResponse(std::move(responsePtr));
}

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic