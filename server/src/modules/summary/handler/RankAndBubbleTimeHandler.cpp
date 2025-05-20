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
    auto &request = dynamic_cast<PipelineRankTimeRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<PipelineRankTimeResponse> responsePtr = std::make_unique<PipelineRankTimeResponse>();
    PipelineRankTimeResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    // check request parameters
    std::string errorMsg;
    if (!request.params.CheckParams(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(request.params.clusterPath);
    if (database == nullptr || !database->GetRankAndBubble(request.params, response.body)) {
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