/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "RankAndBubbleTimeHandler.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic::Server;
bool RankAndBubbleTimeHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    PipelineRankTimeRequest &request = dynamic_cast<PipelineRankTimeRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<PipelineRankTimeResponse> responsePtr = std::make_unique<PipelineRankTimeResponse>();
    PipelineRankTimeResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    auto database = Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    if (!database->GetRankAndBubble(request.params, response.body)) {
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