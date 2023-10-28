/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "StageAndBubbleTimeHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic::Server;
void StageAndBubbleTimeHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    PipelineStageTimeRequest &request = dynamic_cast<PipelineStageTimeRequest &>(*requestPtr.get());
    if (!WsSessionManager::Instance().CheckSession(request.token)) {
        ServerLog::Error("Failed to check session token , command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(request.token);
    std::unique_ptr<PipelineStageTimeResponse> responsePtr = std::make_unique<PipelineStageTimeResponse>();
    PipelineStageTimeResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase();
    if (!database->GetStageAndBubble(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get time response data.");
    }
    session.OnResponse(std::move(responsePtr));
}

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic