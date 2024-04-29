/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "WsSessionManager.h"
#include "TraceTime.h"
#include "DataBaseManager.h"
#include "QueryFlowCategoryEventsHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void QueryFlowCategoryEventsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    FlowCategoryEventsRequest &request = dynamic_cast<FlowCategoryEventsRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<FlowCategoryEventsResponse> responsePtr = std::make_unique<FlowCategoryEventsResponse>();
    FlowCategoryEventsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", request.params.rankId);
        SetResponseResult(response, true);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    bool result = database->QueryFlowCategoryEvents(request.params, TraceTime::Instance().GetStartTime(),
                                                    response.body.flowDetailList);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // Timeline
} // Module
} // Dic