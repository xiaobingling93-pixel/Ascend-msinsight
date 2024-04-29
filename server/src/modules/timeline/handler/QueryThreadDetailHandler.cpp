//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#include "QueryThreadDetailHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceFileParser.h"
#include "TraceTime.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void QueryThreadDetailHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ThreadDetailRequest &request = dynamic_cast<ThreadDetailRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<UnitThreadDetailResponse> responsePtr = std::make_unique<UnitThreadDetailResponse>();
    UnitThreadDetailResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", request.params.rankId);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    int64_t trackId = TraceFileParser::Instance()
        .GetTrackId(request.params.rankId, request.params.pid, request.params.tid);
    bool result = database->QueryThreadDetail(request.params, response.body, TraceTime::Instance().GetStartTime(),
                                              trackId);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Timeline
} // Module
} // Dic
