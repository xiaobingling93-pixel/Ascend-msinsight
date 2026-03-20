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
    std::unique_ptr<SearchAllSlicesResponse> responsePtr = std::make_unique<SearchAllSlicesResponse>();
    SearchAllSlicesResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    std::string warnMsg;
    uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    if (!request.params.CheckParams(minTimestamp, warnMsg)) {
        ServerLog::Warn(warnMsg);
        SetTimelineError(ErrorCode::PARAMS_ERROR);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get search all slices  connection.");
        SetTimelineError(ErrorCode::CONNECT_DATABASE_FAILED);
        SendResponse(std::move(responsePtr), false);
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
        SetTimelineError(ErrorCode::QUERY_SLICE_DETAIL_FAILED);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    response.body.dbPath = database->GetDbPath();
    SendResponse(std::move(responsePtr), true);

    return true;
}

} // Timeline
} // Module
} // Dic
