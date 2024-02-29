/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "MatrixSortOpNamesHandler.h"
#include "ServerLog.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic;
using namespace Dic::Server;

void MatrixSortOpNamesHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    MatrixSortOpNamesRequest &request = dynamic_cast<MatrixSortOpNamesRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Error("Failed to check session token , command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<MatrixSortOpNamesResponse> responsePtr = std::make_unique<MatrixSortOpNamesResponse>();
    MatrixSortOpNamesResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    if (!database->QueryMatrixSortOpNames(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get matrix sort op names response data.");
        session.OnResponse(std::move(responsePtr));
        return;
    }
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Communication
} // Module
} // Dic