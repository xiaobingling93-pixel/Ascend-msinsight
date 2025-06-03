//
// * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TrackInfoManager.h"
#include "TraceTime.h"
#include "QueryFlowsBySliceInfoHandler.h"
namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
bool QueryFlowsBySliceInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    UnitFlowsRequest &request = dynamic_cast<UnitFlowsRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<UnitFlowsResponse> responsePtr = std::make_unique<UnitFlowsResponse>();
    UnitFlowsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    std::string warnMsg;
    if (!request.params.CheckParams(minTimestamp, warnMsg)) {
        ServerLog::Warn(warnMsg);
        SetResponseResult(response, false, warnMsg);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Query flows by slice info failed to get connection. ");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    uint64_t trackId =
        TrackInfoManager::Instance().GetTrackId(request.params.rankId, request.params.pid, request.params.tid);
    bool result;
    try {
        result = database->QueryUintFlows(request.params, response.body, minTimestamp, trackId);
    }  catch (DatabaseException &e) {
        e.Log("Query flows by slice info Fail, ");
    }

    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return result;
}
} // Timeline
} // Module
} // Dic