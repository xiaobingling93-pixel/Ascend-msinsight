/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "QueryComputeDetailInfoHandler.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic::Server;
bool QueryComputeDetailInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ComputeDetailRequest &request = dynamic_cast<ComputeDetailRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<ComputeDetailResponse> responsePtr = std::make_unique<ComputeDetailResponse>();
    ComputeDetailResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        ServerLog::Error("[Operator]Failed to check request parameter.", errorMsg);
        SetResponseResult(response, false, errorMsg);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(request.params.rankId);
    if (!database->QueryComputeOpDetail(request.params, response.computeDetails) or
        !database->QueryTotalNumByAcceleratorCore(request.params.timeFlag, response.totalNum)) {
        ServerLog::Warn("Query compute detail or query total num is failed");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // Summary
} // Module
} // Dic