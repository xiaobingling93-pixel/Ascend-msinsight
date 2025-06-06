/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "TrackInfoManager.h"
#include "SearchAllSlicesHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
bool SearchAllSlicesHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    SearchAllSlicesRequest &request = dynamic_cast<SearchAllSlicesRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<SearchAllSlicesResponse> responsePtr = std::make_unique<SearchAllSlicesResponse>();
    SearchAllSlicesResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    std::string warnMsg;
    uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    if (!request.params.CheckParams(minTimestamp, warnMsg)) {
        ServerLog::Warn(warnMsg);
        SetResponseResult(response, false, warnMsg);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get search all slices  connection.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    request.params.fileId = request.fileId;
    std::vector<TrackQuery> trackQueryVec;
    for (const auto &item : request.params.metadataList) {
        if ((request.params.rankId == item.rankId || item.rankId == database->GetDbPath()) && !item.pid.empty() &&
            !item.tid.empty()) {
            TrackQuery trackQuery;
            trackQuery.rankId = request.params.rankId;
            trackQuery.processId = item.pid;
            trackQuery.threadId = item.tid;
            trackQuery.trackId = TrackInfoManager::Instance().GetTrackId(request.params.rankId, item.pid, item.tid);
            trackQuery.startTime = item.lockStartTime + minTimestamp; // 校验过，保证 lockStartTime < lockEndTime
            trackQuery.endTime = item.lockEndTime + minTimestamp; // 校验过，保证 lockEndTime + minTime < UINT64_MAX
            trackQuery.metaType = item.metaType;
            trackQueryVec.emplace_back(trackQuery);
        }
    }
    if (!database->SearchAllSlicesDetails(request.params, response.body, minTimestamp, trackQueryVec)) {
        ServerLog::Error("Failed to search slice details.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    response.body.dbPath = database->GetDbPath();
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // Timeline
} // Module
} // Dic