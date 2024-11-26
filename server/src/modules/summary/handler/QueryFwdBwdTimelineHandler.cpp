/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <algorithm>
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "TraceDatabaseHelper.h"
#include "QueryFwdBwdTimelineHandler.h"

namespace Dic::Module::Summary {
using namespace Dic::Server;
using namespace Dic::Module::Timeline;
bool QueryFwdBwdTimelineHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<PipelineFwdBwdTimelineRequest &>(*requestPtr.get());
    std::unique_ptr<PipelineFwdBwdTimelineResponse> responsePtr = std::make_unique<PipelineFwdBwdTimelineResponse>();
    PipelineFwdBwdTimelineResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    // check request parameter
    std::string err;
    if (!request.params.CheckParams(err)) {
        SendResponse(std::move(responsePtr), false, "Failed to query fwd/bwd timeline data due to error parameters.");
        return false;
    }
    std::vector<std::string> rankIds = StringUtil::SplitStringWithParenthesesByComma(request.params.stageId);
    if (rankIds.empty()) {
        SendResponse(std::move(responsePtr), false, "Failed to query fwd/bwd timeline data due to empty rank ids.");
        return false;
    }
    for (auto const &rankId : rankIds) {
        PipelineFwdBwdTimelineByRank data = {rankId, {}, {}};
        if (!QueryFwdBwdTimelineByRank(rankId, request.params.stepId, data, response.body)) {
            ServerLog::Warn("Failed to query fwd/bwd timeline data for rand ", rankId);
            data = {rankId, {}, {}};
        }
        response.body.rankLists.push_back(rankId);
        response.body.rankDataList.push_back(data);
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}

// responseBody only for max/min time
bool QueryFwdBwdTimelineHandler::QueryFwdBwdTimelineByRank(const std::string &rankId, const std::string &stepId,
    PipelineFwdBwdTimelineByRank &data, PipelineFwdBwdTimelineResponseBody &responseBody)
{
    uint64_t offset = Timeline::TraceTime::Instance().GetStartTime();
    auto database = DataBaseManager::Instance().GetTraceDatabase(rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to query fwd/bwd timeline data by rank due to null connection for database.");
        return false;
    }
    Protocol::ExtremumTimestamp range = {offset, (uint64_t)INT64_MAX};
    // 此处不检查返回值，原因为如果查询失败，则以offset为最小值，INT64_MAX为最大值，对应ALL场景
    database->QueryStepDuration(stepId, range.minTimestamp, range.maxTimestamp);

    data.rankId = rankId;

    // 组装前反向数据
    PipelineFwdBwdTimelineByComponent fwdBwdData = {LANE_FP_BP, {}};
    if (!database->QueryFwdBwdDataByFlow(rankId, offset, range, fwdBwdData.traceList)) {
        ServerLog::Warn("Failed to query fwd/bwd detail trace data for rank ", rankId);
    }

    if (!fwdBwdData.traceList.empty()) {
        auto first = fwdBwdData.traceList.at(0);
        auto last = fwdBwdData.traceList.at(fwdBwdData.traceList.size() - 1);
        responseBody.maxTime = std::max(responseBody.maxTime, last.startTime + last.duration);
        responseBody.minTime = std::min(responseBody.minTime, first.startTime);
    }

    data.componentDataList.push_back(fwdBwdData);

    // 组装P2P算子数据，再另一个MR里，带合并后补充此部分
    PipelineFwdBwdTimelineByComponent p2pOpData = {LANE_P2P_OP, {}};
    if (!database->QueryP2PCommunicationOpData(rankId, offset, range, p2pOpData.traceList)) {
        ServerLog::Warn("Failed to query p2p operator detail for rank ", rankId);
    }
    if (!p2pOpData.traceList.empty()) {
        auto first = p2pOpData.traceList.at(0);
        auto last = p2pOpData.traceList.at(p2pOpData.traceList.size() - 1);
        responseBody.maxTime = std::max(responseBody.maxTime, last.startTime + last.duration);
        responseBody.minTime = std::min(responseBody.minTime, first.startTime);
    }
    data.componentDataList.push_back(p2pOpData);
    return true;
}
} // Dic::Module::Summary