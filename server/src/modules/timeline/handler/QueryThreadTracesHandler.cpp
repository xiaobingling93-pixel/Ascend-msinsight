//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//
#include "WsSessionManager.h"
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
    if (std::empty(request.params.threadIdList)) {
        uint64_t trackId = TrackInfoManager::Instance().GetTrackId(request.params.cardId, request.params.processId,
                                                                   request.params.threadId);
        renderEngine->QueryThreadTraces(request.params, response.body, minTimestamp, trackId);
    } else {
        QueryTracesByTrackIds(request, response, minTimestamp);
    }
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}

void QueryThreadTracesHandler::QueryTracesByTrackIds(UnitThreadTracesRequest& request,
                                                     UnitThreadTracesResponse& response, uint64_t minTimestamp)
{
    for (const auto& threadId : request.params.threadIdList) {
        UnitThreadTracesBody tempBody;
        request.params.threadId = threadId;
        uint64_t trackId = TrackInfoManager::Instance().GetTrackId(request.params.cardId, request.params.processId,
                                                                   request.params.threadId);
        renderEngine->QueryThreadTraces(request.params, tempBody, minTimestamp, trackId);
        for (size_t i = 0; i < tempBody.data.size(); ++i) {
            while (response.body.data.size() <= i) {
                response.body.data.emplace_back();
            }
            response.body.data[i].insert(response.body.data[i].end(), tempBody.data[i].begin(), tempBody.data[i].end());
        }
        response.body.maxDepth = std::max(tempBody.maxDepth, response.body.maxDepth);
        response.body.currentMaxDepth = std::max(tempBody.currentMaxDepth, response.body.currentMaxDepth);
    }
    for (auto& item : response.body.data) {
        std::sort(item.begin(), item.end(), [](const ThreadTraces& first, const ThreadTraces& second) {
            if (first.startTime != second.startTime) {
                return first.startTime < second.startTime;
            }
            if (first.endTime != second.endTime) {
                return first.endTime < second.endTime;
            }
            return first.id < second.id;
        });
    }
}
}  // namespace Timeline
}  // namespace Module
}  // namespace Dic