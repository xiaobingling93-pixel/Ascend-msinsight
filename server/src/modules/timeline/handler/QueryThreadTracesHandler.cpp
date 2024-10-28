//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TrackInfoManager.h"
#include "TraceTime.h"
#include "QueryThreadTracesHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
bool QueryThreadTracesHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    UnitThreadTracesRequest &request = dynamic_cast<UnitThreadTracesRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<UnitThreadTracesResponse> responsePtr = std::make_unique<UnitThreadTracesResponse>();
    UnitThreadTracesResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    std::string warnMsg;
    if (!request.params.CheckParams(minTimestamp, warnMsg)) {
        ServerLog::Warn(warnMsg);
        SetResponseResult(response, false, warnMsg);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    if (renderEngine == nullptr) {
        ServerLog::Error("Query thread traces Failed to render.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    int64_t trackId = TrackInfoManager::Instance().GetTrackId(request.params.cardId, request.params.processId,
        request.params.threadId);

    renderEngine->QueryThreadTraces(request.params, response.body, minTimestamp, trackId);
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Timeline
} // Module
} // Dic