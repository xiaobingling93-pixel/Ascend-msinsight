/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "QueryComputeDetailInfoHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic::Server;
void QueryComputeDetailInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ComputeDetailRequest &request = dynamic_cast<ComputeDetailRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Error("Failed to check session token , command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<ComputeDetailResponse> responsePtr = std::make_unique<ComputeDetailResponse>();
    ComputeDetailResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        ServerLog::Error("[Operator]Failed to check request parameter.", errorMsg);
        SetResponseResult(response, false, errorMsg);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(request.params.rankId);
    if (!database->QueryComputeDetailHandler(request.params, response.computeDetails) or
        !database->QueryGetTotalNum(request.params.timeFlag, response.totalNum)) {
        ServerLog::Warn("Query compute detail or query total num is failed");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Summary
} // Module
} // Dic