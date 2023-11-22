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
    ServerLog::Info("Query flow category event, token = ", StringUtil::AnonymousString(token));
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<FlowCategoryEventsResponse> responsePtr = std::make_unique<FlowCategoryEventsResponse>();
    FlowCategoryEventsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    bool result = database->QueryFlowCategoryEvents(request.params, TraceTime::Instance().GetStartTime(),
                                                    response.body.flowDetailList);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // Timeline
} // Module
} // Dic