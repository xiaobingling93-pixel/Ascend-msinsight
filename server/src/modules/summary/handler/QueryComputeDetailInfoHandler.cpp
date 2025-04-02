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
    // check request parameters
    std::string errorMsg;
    if (!request.params.CheckParams(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(request.params.rankId);
    if (!database->QueryComputeOpDetail(request.params, response.computeDetails) or
        !database->QueryTotalNumByAcceleratorCore(request.params.timeFlag, response.totalNum)) {
        ServerLog::Warn("Failed to query compute detail or query total num.");
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