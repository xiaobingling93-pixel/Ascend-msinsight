/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/


#include "QueryThreadsSameOperatorHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceFileParser.h"
#include "TraceTime.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void QueryThreadsSameOperatorHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    UnitThreadsOperatorsRequest &request = dynamic_cast<UnitThreadsOperatorsRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Error("Failed to check command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<UnitThreadsOperatorsResponse> responsePtr = std::make_unique<UnitThreadsOperatorsResponse>();
    UnitThreadsOperatorsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    auto db = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (db == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", request.params.rankId);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    auto database = std::dynamic_pointer_cast<JsonTraceDatabase, VirtualTraceDatabase>(db);
    if (database == nullptr) {
        ServerLog::Error("Failed to convert VirtualTraceDatabase to JsonTraceDatabase in same op HandleRequest.");
        return;
    }
    int64_t trackId = TraceFileParser::Instance()
            .GetTrackId(request.params.rankId, request.params.pid, request.params.tid);
    bool result = database->QueryThreadSameOperatorsDetails(request.params, response.body,
                                                            TraceTime::Instance().GetStartTime(), trackId);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Timeline
} // Module
} // Dic