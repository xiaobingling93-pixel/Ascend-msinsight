/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "WsSessionManager.h"
#include "ClusterService.h"
#include "CommunicationSlowRankAnalysisHandler.h"

namespace Dic::Module::Communication {
using namespace Dic::Server;
bool CommunicationSlowRankAnalysisHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<DurationListRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<CommunicationSlowRankAnalysisResponse> responsePtr =
        std::make_unique<CommunicationSlowRankAnalysisResponse>();
    CommunicationSlowRankAnalysisResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    // check request parameters
    std::string errorMsg;
    if (!request.params.CheckParams(errorMsg)) {
        SetCommunicationError(ErrorCode::PARAMS_ERROR);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    if (!ClusterService::AnalyzeCommunicationSlowRanks(request.params, response.body)) {
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}
}
