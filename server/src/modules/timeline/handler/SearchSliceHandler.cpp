/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "TrackInfoManager.h"
#include "SearchSliceHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
bool SearchSliceHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    SearchSliceRequest &request = dynamic_cast<SearchSliceRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<SearchSliceResponse> responsePtr = std::make_unique<SearchSliceResponse>();
    SearchSliceResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    std::string warnMsg;
    if (!request.params.CheckParams(minTimestamp, warnMsg)) {
        ServerLog::Warn(warnMsg);
        SetResponseResult(response, false, warnMsg);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    response.body.rankId = request.params.rankId;
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Search slice can't find rankId.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    std::vector<TrackQuery> trackQueryVec;
    for (const auto &item: request.params.metadataList) {
        if (item.rankId == request.params.rankId && !item.tid.empty() && !item.pid.empty()) {
            TrackQuery trackQuery;
            trackQuery.rankId = item.rankId;
            trackQuery.processId = item.pid;
            trackQuery.threadId = item.tid;
            trackQuery.startTime = item.lockStartTime + minTimestamp;
            trackQuery.endTime = item.lockEndTime + minTimestamp;
            trackQuery.trackId = TrackInfoManager::Instance().GetTrackId(item.rankId, item.pid, item.tid);
            trackQuery.metaType = item.metaType;
            trackQueryVec.emplace_back(trackQuery);
        }
    }
    if (!database->SearchSliceName(request.params, request.params.index - 1,
                                   minTimestamp, response.body, trackQueryVec)) {
        ServerLog::Error("Failed to search slice name.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // Timeline
} // Module
} // Dic