//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#include "QueryThreadTracesHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceFileParser.h"
#include "TraceTime.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void QueryThreadTracesHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    UnitThreadTracesRequest &request = dynamic_cast<UnitThreadTracesRequest &>(*requestPtr.get());
    std::string token = request.token;
    ServerLog::Info("Query thread traces, token = ", StringUtil::AnonymousString(token));
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<UnitThreadTracesResponse> responsePtr = std::make_unique<UnitThreadTracesResponse>();
    UnitThreadTracesResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.cardId);
    int64_t trackId = TraceFileParser::Instance()
            .GetTrackId(request.params.cardId, request.params.processId, request.params.threadId);
    if (!database->QueryThreadTraces(request.params, response.body, TraceTime::Instance().GetStartTime(), trackId)) {
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