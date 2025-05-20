/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "MatrixSortOpNamesHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic;
using namespace Dic::Server;

bool MatrixSortOpNamesHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<MatrixSortOpNamesRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<MatrixSortOpNamesResponse> responsePtr = std::make_unique<MatrixSortOpNamesResponse>();
    MatrixSortOpNamesResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    // check request parameters
    std::string errorMsg;
    if (!request.params.CheckParams(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(request.params.clusterPath);
    if (database == nullptr || !database->QueryMatrixSortOpNames(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get matrix sort op names response data.");
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