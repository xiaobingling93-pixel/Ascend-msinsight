//
// * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceFileParser.h"
#include "TraceTime.h"
#include "QueryFlowsBySliceInfoHandler.h"
namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void QueryFlowsBySliceInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    UnitFlowsRequest &request = dynamic_cast<UnitFlowsRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<UnitFlowsResponse> responsePtr = std::make_unique<UnitFlowsResponse>();
    UnitFlowsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", request.params.rankId);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    uint64_t trackId =
        TraceFileParser::Instance().GetTrackId(request.params.rankId, request.params.pid, request.params.tid);
    bool result =
        database->QueryUintFlows(request.params, response.body, TraceTime::Instance().GetStartTime(), trackId);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // Timeline
} // Module
} // Dic