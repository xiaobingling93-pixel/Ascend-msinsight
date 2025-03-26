/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "pch.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "RanksHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic;
using namespace Dic::Server;
bool RanksHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    RanksRequest &request = dynamic_cast<RanksRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<RanksResponse> responsePtr = std::make_unique<RanksResponse>();
    RanksResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    // check request parameters
    std::string errorMsg;
    if (!request.params.CheckParams(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr || !database->QueryRanksHandler(response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get ranks response data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // Communication
} // Module
} // Dic