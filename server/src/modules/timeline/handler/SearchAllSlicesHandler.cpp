/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "SearchAllSlicesHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void SearchAllSlicesHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    SearchAllSlicesRequest &request = dynamic_cast<SearchAllSlicesRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<SearchAllSlicesResponse> responsePtr = std::make_unique<SearchAllSlicesResponse>();
    SearchAllSlicesResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get search all slices  connection.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    if (!database->SearchAllSlicesDetails(request.params, response.body, TraceTime::Instance().GetStartTime())) {
        ServerLog::Error("Failed to search slice details.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }

    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Timeline
} // Module
} // Dic