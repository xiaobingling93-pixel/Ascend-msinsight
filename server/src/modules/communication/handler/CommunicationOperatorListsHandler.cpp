/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "CommunicationOperatorListsHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic::Server;
bool CommunicationOperatorListsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    DurationListRequest &request = dynamic_cast<DurationListRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<OperatorListsResponse> responsePtr = std::make_unique<OperatorListsResponse>();
    OperatorListsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    if (!database->QueryOperatorList(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get communication operator list data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Dic
} // Module
} // Communication