/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "pch.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "DistributionHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic;
using namespace Dic::Server;

bool DistributionHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request =
            dynamic_cast<Protocol::DistributionDataRequest &>(*requestPtr);
    std::unique_ptr<Protocol::DistributionResponse> responsePtr =
            std::make_unique<Protocol::DistributionResponse>();
    DistributionResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    // check request parameters
    std::string errorMsg;
    if (!request.params.CheckParams(errorMsg)) {
        SetCommunicationError(ErrorCode::PARAMS_ERROR);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(request.params.clusterPath);
    if (database == nullptr || !database->QueryDistributionData(request.params, response.body)) {
        SetCommunicationError(ErrorCode::QUERY_COMMUNICATION_DISTRIBUTION_FAILED);
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get communication distribution data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Communication
} // Module
} // Dic