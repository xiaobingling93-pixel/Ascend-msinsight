/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "WsSessionManager.h"
#include "TraceTime.h"
#include "DataBaseManager.h"
#include "QueryFlowCategoryEventsHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
bool QueryFlowCategoryEventsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    FlowCategoryEventsRequest &request = dynamic_cast<FlowCategoryEventsRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<FlowCategoryEventsResponse> responsePtr = std::make_unique<FlowCategoryEventsResponse>();
    FlowCategoryEventsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    std::string warnMsg;
    if (!request.params.CheckParams(minTimestamp, warnMsg)) {
        ServerLog::Warn(warnMsg);
        SetResponseResult(response, false, warnMsg);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    if (renderEngine == nullptr) {
        ServerLog::Error("Query flow events Failed to render.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    bool result = renderEngine->QueryFlowCategoryEvents(request.params, minTimestamp, response.body.flowDetailList);
    SetResponseResult(response, result);
    session.OnResponse(std::move(responsePtr));
    return result;
}
} // Timeline
} // Module
} // Dic