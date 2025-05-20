/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "pch.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "BaselineManager.h"
#include "CommunicationOperatorDetailsHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic;
using namespace Dic::Server;
using namespace Dic::Module::Global;

bool CommunicationOperatorDetailsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request =
            dynamic_cast<Protocol::OperatorDetailsRequest &>(*requestPtr);
    std::unique_ptr<Protocol::OperatorDetailsResponse> responsePtr =
            std::make_unique<Protocol::OperatorDetailsResponse>();
    // check request parameters
    std::string errorMsg;
    if (!request.params.CheckParams(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    OperatorDetailsResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    // add response to response queue in session
    WsSession &session = *WsSessionManager::Instance().GetSession();
    // query data
    std::string type = request.params.queryType == "Comparison" ? request.params.clusterPath
                                                                : BaselineManager::Instance().GetBaseLineClusterPath();
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(type);
    if (database == nullptr || !database->QueryOperatorsCount(request.params, response.body) ||
        !database->QueryAllOperators(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get communication operator data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Communication
} // Module
} // Dic