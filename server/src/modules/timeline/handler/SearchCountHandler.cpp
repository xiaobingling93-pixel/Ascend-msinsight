/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "DominQuery.h"
#include "TrackInfoManager.h"
#include "TraceTime.h"
#include "SearchCountHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
bool SearchCountHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    SearchCountRequest &request = dynamic_cast<SearchCountRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<SearchCountResponse> responsePtr = std::make_unique<SearchCountResponse>();
    SearchCountResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    std::string warnMsg;
    if (!request.params.CheckParams(minTimestamp, warnMsg)) {
        ServerLog::Warn(warnMsg);
        SetTimelineError(ErrorCode::PARAMS_ERROR);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    std::vector<TrackQuery> trackQueryVec = GetTrackQueryVec(request, minTimestamp);
    if (!request.params.metadataList.empty()) {
        SearchResult searchResult;
        searchResult.rankId = request.params.rankId;
        auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(request.params.rankId);
        if (database != nullptr) {
            searchResult.count = database->SearchSliceNameCount(request.params, trackQueryVec);
            searchResult.dbPath = database->GetDbPath();
        }
        response.body.countList.emplace_back(searchResult);
        response.body.totalCount = searchResult.count;
    } else {
        SearchResult searchResult;
        searchResult.rankId = request.params.rankId;
        auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(request.params.rankId);
        if (database != nullptr) {
            searchResult.count = database->SearchSliceNameCount(request.params, {});
            searchResult.dbPath = database->GetDbPath();
        }
        response.body.countList.emplace_back(searchResult);
        response.body.totalCount = searchResult.count;
    }
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}

std::vector<TrackQuery> SearchCountHandler::GetTrackQueryVec(SearchCountRequest &request, uint64_t minTimestamp) const
{
    std::vector<TrackQuery> trackQueryVec;
    for (const auto &item : request.params.metadataList) {
        if ((request.params.rankId == item.rankId ||
            !DataBaseManager::Instance().GetDbPathByHost(request.params.rankId).empty()) &&
            !item.pid.empty() && !item.tid.empty()) {
            TrackQuery trackQuery;
            trackQuery.rankId = item.rankId;
            trackQuery.processId = item.pid;
            trackQuery.threadId = item.tid;
            trackQuery.trackId = TrackInfoManager::Instance().GetTrackId(request.params.rankId, item.pid, item.tid);
            trackQuery.startTime = item.lockStartTime + minTimestamp; // 校验过，保证 lockStartTime < lockEndTime
            trackQuery.endTime = item.lockEndTime + minTimestamp; // 校验过，保证 lockEndTime + minTime < UINT64_MAX
            trackQuery.metaType = item.metaType;
            trackQueryVec.emplace_back(trackQuery);
        }
    }
    return trackQueryVec;
}
} // Timeline
} // Module
} // Dic