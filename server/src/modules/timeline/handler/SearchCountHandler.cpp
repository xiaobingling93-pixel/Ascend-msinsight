/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
        SetResponseResult(response, false, warnMsg);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    std::vector<TrackQuery> trackQueryVec = GetTrackQueryVec(request, minTimestamp);
    if (request.params.rankId.empty() || !DataBaseManager::Instance().GetDbPathByHost(request.params.rankId).empty()) {
        QueryHostNameCount(request, response, trackQueryVec);
    } else if (!request.params.metadataList.empty()) {
        SearchResult searchResult;
        searchResult.rankId = request.params.rankId;
        auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(request.params.rankId);
        if (database != nullptr) {
            searchResult.count = database->SearchSliceNameCount(request.params, trackQueryVec);
        }
        response.body.countList.emplace_back(searchResult);
        response.body.totalCount = searchResult.count;
    } else {
        SearchResult searchResult;
        searchResult.rankId = request.params.rankId;
        auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(request.params.rankId);
        if (database != nullptr) {
            searchResult.count = database->SearchSliceNameCount(request.params, {});
        }
        response.body.countList.emplace_back(searchResult);
        response.body.totalCount = searchResult.count;
    }
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}

void SearchCountHandler::QueryHostNameCount(const SearchCountRequest &request, SearchCountResponse &response,
    const std::vector<TrackQuery> &trackQueryVec) const
{
    auto fileIdList = DataBaseManager::Instance().GetDbPathByHost(request.params.rankId);
    for (const auto &fileId : fileIdList) {
        ServerLog::Info("request.params.rankId is: ", fileId);
        SearchResult searchResult;
        searchResult.rankId = fileId;
        auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(fileId);
        if (database != nullptr) {
            searchResult.count = database->SearchSliceNameCount(request.params, trackQueryVec);
        }
        response.body.totalCount += searchResult.count;
        if (searchResult.count > 0) {
            response.body.countList.emplace_back(searchResult);
        }
    }
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