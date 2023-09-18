/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "StageHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic::Server;
void StageHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    PipelineStageRequest &request = dynamic_cast<PipelineStageRequest &>(*requestPtr.get());
    if (!WsSessionManager::Instance().CheckSession(request.token)) {
        ServerLog::Warn("Failed to check session command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(request.token);
    std::unique_ptr<PipelineStageResponse> responsePtr = std::make_unique<PipelineStageResponse>();
    PipelineStageResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase();
    if (!database->GetStages(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get stage response data.");
    }
    session.OnResponse(std::move(responsePtr));
}

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic