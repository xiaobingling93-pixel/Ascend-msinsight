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
std::map<std::string, PipelineFwdBwdTimelineByRank> QueryFwdBwdTimelineHandler::dataMap;
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
    dataMap.clear();
    ThreadPool threadPool = ThreadPool(4); // thread pool count 4
    for (auto const &rankId : rankIds) {
        response.body.rankLists.push_back(rankId);
        PipelineFwdBwdTimelineByRank rank = {rankId, {}, {}};
        dataMap.emplace(rankId, rank);
        threadPool.AddTask(QueryFwdBwdTimelineByRank, rankId, request.params.stepId, request.params.clusterPath);
    }

    threadPool.WaitForAllTasks();
    threadPool.ShutDown();

    // collect all data
    for (auto const &rankId : rankIds) {
        response.body.rankDataList.push_back(dataMap[rankId]);
    }

    // calculate max/min timestamp, < 100 ranks, controllable time consumption
    for (auto &rank : response.body.rankDataList) {
        if (rank.componentDataList.empty()) {
            continue; // empty rank
        }
        for (auto &component : rank.componentDataList) {
            if (component.traceList.empty()) {
                continue; // empty component
            }
            auto first = component.traceList.at(0);
            auto last = component.traceList.at(component.traceList.size() - 1);
            response.body.maxTime = std::max(response.body.maxTime, last.startTime + last.duration);
            response.body.minTime = std::min(response.body.minTime, first.startTime);
        }
    }
    SendResponse(std::move(responsePtr), true);
    dataMap.clear();
    return true;
}

bool QueryFwdBwdTimelineHandler::QueryFwdBwdTimelineByRank(const std::string &rankId, const std::string &stepId,
                                                           const std::string &clusterPath)
{
    if (dataMap.find(rankId) == dataMap.end()) {
        return false;
    }
    auto rank = &dataMap.at(rankId);
    uint64_t offset = Timeline::TraceTime::Instance().GetStartTime();
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(rankId);
    if (database == nullptr) {
        // DB场景下无法根据rankId获取database
        database = DataBaseManager::Instance().GetTraceDatabaseInCluster(clusterPath, rankId);
        if (database == nullptr) {
            ServerLog::Error("Failed to query fwd/bwd timeline data by rank due to null connection for database.");
            return false;
        }
    }
    Protocol::ExtremumTimestamp range = {offset, (uint64_t)INT64_MAX};
    // 此处不检查返回值，原因为如果查询失败，则以offset为最小值，INT64_MAX为最大值，对应ALL场景
    database->QueryStepDuration(stepId, range.minTimestamp, range.maxTimestamp);
    rank->rankId = rankId;

    // 组装前反向数据
    PipelineFwdBwdTimelineByComponent fwdBwdData = {LANE_FP_BP, {}};
    if (!database->QueryFwdBwdDataByFlow(rankId, offset, range, fwdBwdData.traceList)) {
        ServerLog::Warn("Failed to query fwd/bwd detail trace data for rank ", rankId);
    }
    rank->componentDataList.push_back(fwdBwdData);

    // 组装P2P算子数据，再另一个MR里，带合并后补充此部分
    PipelineFwdBwdTimelineByComponent p2pOpData = {LANE_P2P_OP, {}};
    if (!database->QueryP2PCommunicationOpData(rankId, offset, range, p2pOpData.traceList)) {
        ServerLog::Warn("Failed to query p2p operator detail for rank ", rankId);
    }
    rank->componentDataList.push_back(p2pOpData);
    return true;
}
} // Dic::Module::Summary