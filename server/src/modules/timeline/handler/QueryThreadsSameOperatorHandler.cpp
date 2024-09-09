/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TrackInfoManager.h"
#include "TraceTime.h"
#include "QueryThreadsSameOperatorHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void QueryThreadsSameOperatorHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    UnitThreadsOperatorsRequest &request = dynamic_cast<UnitThreadsOperatorsRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<UnitThreadsOperatorsResponse> responsePtr = std::make_unique<UnitThreadsOperatorsResponse>();
    UnitThreadsOperatorsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    auto db = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (db == nullptr) {
        ServerLog::Error("Query threads same operator failed to get connection.");
        session.OnResponse(std::move(responsePtr));
        return;
    }
    int64_t trackId = TrackInfoManager::Instance()
            .GetTrackId(request.params.rankId, request.params.pid, request.params.tid);
    bool result = db->QueryThreadSameOperatorsDetails(request.params, response.body,
                                                      TraceTime::Instance().GetStartTime(), trackId);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Timeline
} // Module
} // Dic