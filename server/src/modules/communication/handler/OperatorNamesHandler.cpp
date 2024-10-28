/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "pch.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "OperatorNamesHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic;
using namespace Dic::Server;

bool OperatorNamesHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    OperatorNamesRequest &request = dynamic_cast<OperatorNamesRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<OperatorNamesResponse> responsePtr = std::make_unique<OperatorNamesResponse>();
    OperatorNamesResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    if (!database->QueryOperatorNames(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get operator names response data.");
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