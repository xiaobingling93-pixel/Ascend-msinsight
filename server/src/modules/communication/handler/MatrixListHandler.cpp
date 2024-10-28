/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "pch.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "MatrixListHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic;
using namespace Dic::Server;

bool MatrixListHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ServerLog::Info("request to Communication Matrix List");
    MatrixBandwidthRequest &request =
            dynamic_cast<MatrixBandwidthRequest &>(*requestPtr.get());
    std::unique_ptr<Protocol::MatrixListResponse> responsePtr =
            std::make_unique<Protocol::MatrixListResponse>();
    MatrixListResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    // add response to response queue in session
    WsSession &session = *WsSessionManager::Instance().GetSession();
    // query data
    auto database = Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    if (!database->QueryMatrixList(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get matrix response data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Communication
} // Module
} // Dic