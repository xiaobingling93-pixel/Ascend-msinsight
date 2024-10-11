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
void QueryCommunicationDetailHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    CommunicationDetailRequest &request = dynamic_cast<CommunicationDetailRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<CommunicationDetailResponse> responsePtr = std::make_unique<CommunicationDetailResponse>();
    CommunicationDetailResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        ServerLog::Error("[Operator]Failed to check request parameter.", errorMsg);
        SetResponseResult(response, false, errorMsg);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(request.params.rankId);
    if (!database->QueryCommunicationOpDetail(request.params, response.commDetails) or
        !database->QueryTotalNumByAcceleratorCore(request.params.timeFlag, response.totalNum)) {
        ServerLog::Warn("query communication detail or get total num is failed");
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