/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceFileParser.h"
#include "TraceTime.h"
#include "QueryThreadTracesSummaryHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void QueryThreadTracesSummaryHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    UnitThreadTracesSummaryRequest &request = dynamic_cast<UnitThreadTracesSummaryRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<UnitThreadTracesSummaryResponse> responsePtr = std::make_unique<UnitThreadTracesSummaryResponse>();
    UnitThreadTracesSummaryResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.cardId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", request.params.cardId);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    bool result = database->QueryThreadTracesSummary(request.params, response.body,
                                                     TraceTime::Instance().GetStartTime());
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Timeline
} // Module
} // Dic