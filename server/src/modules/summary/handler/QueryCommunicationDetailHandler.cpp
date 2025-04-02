/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "QueryCommunicationDetailHandler.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic::Server;
bool QueryCommunicationDetailHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    CommunicationDetailRequest &request = dynamic_cast<CommunicationDetailRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<CommunicationDetailResponse> responsePtr = std::make_unique<CommunicationDetailResponse>();
    CommunicationDetailResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    // check request parameters
    std::string errorMsg;
    if (!request.params.CheckParams(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(request.params.rankId);
    if (!database->QueryCommunicationOpDetail(request.params, response.commDetails) or
        !database->QueryTotalNumByAcceleratorCore(request.params.timeFlag, response.totalNum)) {
        ServerLog::Warn("Failed to query communication detail or get total num.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // Summary
} // Module
} // Dic