/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "QueryEventsViewHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

void QueryEventsViewHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    EventsViewRequest &request = dynamic_cast<EventsViewRequest &>(*requestPtr.get());

    std::unique_ptr<EventsViewResponse> responsePtr = std::make_unique<EventsViewResponse>();
    EventsViewResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Query events view failed to get connection.");
        session.OnResponse(std::move(responsePtr));
        return;
    }
    if (!database->QueryEventsViewData(request.params, responsePtr->body,
        TraceTime::Instance().GetStartTime())) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get events view table response data.");
    }
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic