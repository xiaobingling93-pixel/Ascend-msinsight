/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "ClusterService.h"
#include "CommunicationOperatorListsHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic::Server;
bool CommunicationOperatorListsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto& request = dynamic_cast<DurationListRequest&>(*requestPtr);
    WsSession& session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<OperatorListsResponse> responsePtr = std::make_unique<OperatorListsResponse>();
    OperatorListsResponse& response = *responsePtr;
    SetBaseResponse(request, response);
    // check request parameters
    std::string errorMsg;
    if (!request.params.CheckParams(errorMsg)) {
        SetCommunicationError(ErrorCode::PARAMS_ERROR);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    ClusterService::QueryOperatorList(request.params, response.body);
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}
}  // namespace Communication
}  // namespace Module
}  // namespace Dic