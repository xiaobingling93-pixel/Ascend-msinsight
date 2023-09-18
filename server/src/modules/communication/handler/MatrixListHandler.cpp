/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "MatrixListHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic;
using namespace Dic::Server;

void MatrixListHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ServerLog::Info("request to Communication Matrix List");
    MatrixBandwidthRequest &request =
            dynamic_cast<MatrixBandwidthRequest &>(*requestPtr.get());
    if (!WsSessionManager::Instance().CheckSession(request.token)) {
        ServerLog::Error("Failed to check session token  , command = ", command);
        return;
    }
    std::unique_ptr<Protocol::MatrixListResponse> responsePtr =
            std::make_unique<Protocol::MatrixListResponse>();
    MatrixListResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    // add response to response queue in session
    WsSession &session = *WsSessionManager::Instance().GetSession(request.token);
    // query data
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase();
    if (!database->QueryMatrixList(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get matrix response data.");
    }
    session.OnResponse(std::move(responsePtr));
}
} // Communication
} // Module
} // Dic