/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "StageAndBubbleTimeHandler.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic::Server;
bool StageAndBubbleTimeHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    PipelineStageTimeRequest &request = dynamic_cast<PipelineStageTimeRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<PipelineStageTimeResponse> responsePtr = std::make_unique<PipelineStageTimeResponse>();
    PipelineStageTimeResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    auto database = Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    if (!database->GetStageAndBubble(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get time response data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic