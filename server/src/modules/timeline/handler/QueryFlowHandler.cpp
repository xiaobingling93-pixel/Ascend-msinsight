//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#include "QueryFlowHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "TraceTime.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void QueryFlowHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) {
    UnitFlowRequest &request = dynamic_cast<UnitFlowRequest &>(*requestPtr.get());
    std::string token = request.token;
    ServerLog::Info("Query flow, token = ", StringUtil::AnonymousString(token));
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<UnitFlowResponse> responsePtr = std::make_unique<UnitFlowResponse>();
    UnitFlowResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (!database->QueryFlowDetail(request.params, response.body, TraceTime::Instance().GetStartTime())) {
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
