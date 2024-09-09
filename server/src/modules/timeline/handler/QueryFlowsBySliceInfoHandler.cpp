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
void QueryFlowsBySliceInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    UnitFlowsRequest &request = dynamic_cast<UnitFlowsRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<UnitFlowsResponse> responsePtr = std::make_unique<UnitFlowsResponse>();
    UnitFlowsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Query flows by slice info failed to get connection. ");
        session.OnResponse(std::move(responsePtr));
        return;
    }
    uint64_t trackId =
        TrackInfoManager::Instance().GetTrackId(request.params.rankId, request.params.pid, request.params.tid);
    bool result;
    try {
        result = database->QueryUintFlows(request.params, response.body, TraceTime::Instance().GetStartTime(), trackId);
    }  catch (DatabaseException &e) {
        e.Log("Query flows by slice info Fail, ");
        result = false;
    }

    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // Timeline
} // Module
} // Dic