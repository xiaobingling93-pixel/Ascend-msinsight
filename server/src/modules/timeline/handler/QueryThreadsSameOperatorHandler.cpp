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
bool QueryThreadsSameOperatorHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    UnitThreadsOperatorsRequest &request = dynamic_cast<UnitThreadsOperatorsRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<UnitThreadsOperatorsResponse> responsePtr = std::make_unique<UnitThreadsOperatorsResponse>();
    UnitThreadsOperatorsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    std::string warnMsg;
    if (!request.params.CheckParams(minTimestamp, warnMsg)) {
        ServerLog::Warn(warnMsg);
        SetResponseResult(response, false, warnMsg);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    auto db = DataBaseManager::Instance().GetTraceDatabaseByRankId(request.params.rankId);
    if (db == nullptr) {
        ServerLog::Error("Query threads same operator failed to get connection.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    std::vector<std::string> trackIdList;
    for (const auto& tid : request.params.tid) {
        trackIdList.emplace_back(std::to_string(TrackInfoManager::Instance()
            .GetTrackId(request.params.rankId, request.params.pid, tid)));
    }
    bool result = db->QueryThreadSameOperatorsDetails(request.params, response.body, minTimestamp, trackIdList);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return result;
}

} // Timeline
} // Module
} // Dic