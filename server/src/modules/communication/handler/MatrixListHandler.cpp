/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "pch.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "ClusterService.h"
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
    // check request parameters
    std::string errorMsg;
    if (!request.params.CheckParams(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    // query data
    ClusterService::QueryMatrixInfo(request.params, response.body);
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Communication
} // Module
} // Dic