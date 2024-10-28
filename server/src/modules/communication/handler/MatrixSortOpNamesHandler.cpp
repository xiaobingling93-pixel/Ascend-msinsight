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
    MatrixSortOpNamesRequest &request = dynamic_cast<MatrixSortOpNamesRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<MatrixSortOpNamesResponse> responsePtr = std::make_unique<MatrixSortOpNamesResponse>();
    MatrixSortOpNamesResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    if (!database->QueryMatrixSortOpNames(request.params, response.body)) {
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