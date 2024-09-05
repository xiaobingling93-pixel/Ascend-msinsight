/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "QuerySystemViewHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

void QuerySystemViewHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    SystemViewRequest &request = dynamic_cast<SystemViewRequest &>(*requestPtr.get());

    std::unique_ptr<SystemViewResponse> responsePtr = std::make_unique<SystemViewResponse>();
    SystemViewResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Query system view failed to get connection.");
        session.OnResponse(std::move(responsePtr));
        return;
    }
    if (!database->QuerySystemViewData(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get timeline table response data.");
    }
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Timeline
} // Module
} // Dic