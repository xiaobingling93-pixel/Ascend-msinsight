//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
//
#include "pch.h"
#include "DomainObject.h"
#include "SliceAnalyzer.h"
#include "FlowAnalyzer.h"
#include "TrackInfoManager.h"
#include "FullDbEnumUtil.h"
#include "RenderEngine.h"
namespace Dic::Module::Timeline {
using namespace Dic::Server;
using namespace Dic::Protocol;
void RenderEngine::SetDataEngineInterface(std::shared_ptr<DataEngineInterface> dataEngineInterface)
{
    dataEngine = dataEngineInterface;
}

void RenderEngine::QueryThreadTraces(const Protocol::UnitThreadTracesParams &requestParams,
    Protocol::UnitThreadTracesBody &responseBody, uint64_t minTimestamp, uint64_t traceId)
{
    SliceQuery sliceQuery;
    sliceQuery.startTime = requestParams.startTime;
    sliceQuery.endTime = requestParams.endTime;
    sliceQuery.minTimestamp = minTimestamp;
    sliceQuery.isFilterPythonFunction = requestParams.isFilterPythonFunction;
    sliceQuery.cat = "python_function";
    sliceQuery.trackId = traceId;
    sliceQuery.rankId = requestParams.cardId;
    sliceQuery.metaType = Protocol::STR_TO_ENUM<PROCESS_TYPE>(requestParams.metaType).value();
    uint64_t maxDepth = 0;
    bool havePythonFunction = false;
    std::set<uint64_t> ids;
    std::map<uint64_t, uint32_t> depthMap;
    std::unique_ptr<SliceAnalyzer> sliceAnalyzerPtr = std::make_unique<SliceAnalyzer>();
    sliceAnalyzerPtr->SetRepository(dataEngine);
    sliceAnalyzerPtr->ComputeScreenSliceIds(sliceQuery, ids, maxDepth, havePythonFunction, depthMap);
    std::vector<CompeteSliceDomain> competeSliceVec;
    std::vector<uint64_t> sliceIds(ids.begin(), ids.end());
    dataEngine->QueryCompeteSliceByIds(sliceQuery, sliceIds, competeSliceVec);
    for (auto &item : competeSliceVec) {
        item.depth = depthMap[item.id];
    }
    std::sort(competeSliceVec.begin(), competeSliceVec.end(), std::less<CompeteSliceDomain>());
    for (auto &item : competeSliceVec) {
        bool isHide = requestParams.isHideFlagEvents && (hideAbleNameSet.find(item.name) != hideAbleNameSet.end());
        if (isHide) {
            continue;
        }
        Protocol::ThreadTraces threadTraces{};
        threadTraces.id = std::to_string(item.id);
        threadTraces.name = item.name;
        if (!(item.endTime >= item.timestamp && item.timestamp >= minTimestamp && item.endTime >= minTimestamp)) {
            continue;
        }
        threadTraces.duration = item.endTime - item.timestamp;
        threadTraces.startTime = item.timestamp - minTimestamp;
        threadTraces.endTime = item.endTime - minTimestamp;
        threadTraces.depth = depthMap[item.id];
        threadTraces.threadId = requestParams.threadId;
        threadTraces.cname = item.cname;
        while (responseBody.data.size() <= item.depth) {
            responseBody.data.emplace_back();
        }
        responseBody.data[item.depth].emplace_back(threadTraces);
    }
    responseBody.maxDepth = maxDepth;
    responseBody.currentMaxDepth = responseBody.data.size();
    responseBody.havePythonFunction = havePythonFunction;
}

bool RenderEngine::QueryFlowCategoryEvents(Protocol::FlowCategoryEventsParams &params, uint64_t minTimestamp,
    std::vector<std::unique_ptr<Protocol::UnitSingleFlow>> &flowDetailList)
{
    std::vector<FlowPoint> flowPointResult;
    std::vector<FlowPoint> flowEventsVec;
    FlowQuery flowQuery;
    flowQuery.cat = params.category;
    flowQuery.fileId = params.rankId;
    flowQuery.minTimestamp = minTimestamp;
    dataEngine->QueryFlowPointByCategory(flowQuery, flowEventsVec);
    flowEventsVec = ComputeLockRangePoints(params, flowEventsVec);
    std::unique_ptr<FlowAnalyzer> flowAnalyzerPtr = std::make_unique<FlowAnalyzer>();
    flowAnalyzerPtr->ComputeScreenFlowPoint(flowEventsVec, params.startTime, params.endTime, flowPointResult);
    std::unique_ptr<SliceAnalyzer> sliceAnalyzerPtr = std::make_unique<SliceAnalyzer>();
    flowAnalyzerPtr->SortByTrackIdASC(flowPointResult);
    ThreadQuery threadQuery;
    threadQuery.fileId = params.rankId;
    TrackInfo trackInfo;
    if (params.isSimulation) {
        ComputeSimulationFlows(params, flowDetailList, flowPointResult);
        return true;
    }
    uint64_t curTrackId = 0;
    std::vector<SliceDomain> cacheSlices;
    for (auto &item : flowPointResult) {
        if (item.trackId != curTrackId) {
            cacheSlices.clear();
            std::string sliceCacheKey = std::to_string(item.trackId);
            cacheSlices = SliceCacheManager::Instance().GetSliceDomainVec(sliceCacheKey);
            curTrackId = item.trackId;
            TrackInfoManager::Instance().GetTrackInfo(curTrackId, trackInfo);
            sliceAnalyzerPtr->SortByTimestampASC(cacheSlices);
        }
        // item.timestamp = timestamp - flowQuery.minTimestamp，timestamp 是从数据库中查出，一定有 timestamp <= INT64_MAX
        // 业务上 flowQuery.minTimestamp 的值能保证是数据库中的最小时间
        item.depth = sliceAnalyzerPtr->ComputeFlowPointDepth(cacheSlices, item.type, item.timestamp + minTimestamp);
        item.pid = trackInfo.processId;
        item.tid = trackInfo.threadId;
    }
    flowAnalyzerPtr->SortByFlowIdAndTimestampASC(flowPointResult);
    flowAnalyzerPtr->ComputeUintFlows(flowPointResult, params.category, flowDetailList);
    ServerLog::Info("Query flow category events. size:", flowDetailList.size());
    return true;
}

std::vector<FlowPoint> RenderEngine::ComputeLockRangePoints(FlowCategoryEventsParams &params,
    std::vector<FlowPoint> &flowEventsVec) const
{
    ServerLog::Info("flowEventsVec size is: ", flowEventsVec.size());
    std::unordered_set<uint64_t> trackIdSet;
    for (const auto &metadata : params.metadataList) {
        if (std::empty(metadata.pid) || std::empty(metadata.tid)) {
            continue;
        }
        uint64_t trackId = TrackInfoManager::Instance().GetTrackId(params.rankId, metadata.pid, metadata.tid);
        trackIdSet.emplace(trackId);
    }
    if (std::empty(trackIdSet)) {
        return flowEventsVec;
    }
    std::unordered_set<std::string> lockFlowIdSet;
    for (const auto &item : flowEventsVec) {
        if (trackIdSet.count(item.trackId) > 0 && item.timestamp >= params.lockStartTime &&
            item.timestamp <= params.lockEndTime) {
            lockFlowIdSet.emplace(item.flowId);
        }
    }
    std::vector<FlowPoint> lockFlowPointVec;
    for (const auto &item : flowEventsVec) {
        if (lockFlowIdSet.count(item.flowId) > 0) {
            lockFlowPointVec.emplace_back(item);
        }
    }
    ServerLog::Info("lockFlowPointVec size is: ", lockFlowPointVec.size());
    return lockFlowPointVec;
}

void RenderEngine::ComputeSimulationFlows(const FlowCategoryEventsParams &params,
    std::vector<std::unique_ptr<Protocol::UnitSingleFlow>> &flowDetailList, std::vector<FlowPoint> &flowPointResult)
{
    TrackInfo trackInfo;
    std::unique_ptr<FlowAnalyzer> flowAnalyzerPtr = std::make_unique<FlowAnalyzer>();
    std::unique_ptr<SliceAnalyzer> sliceAnalyzerPtr = std::make_unique<SliceAnalyzer>();
    std::unordered_map<std::string, uint32_t> simpleSliceMap;
    SliceQuery sliceQuery;
    sliceQuery.rankId = params.rankId;
    uint64_t curTrackId = 0;
    for (auto &item : flowPointResult) {
        if (curTrackId != item.trackId) {
            curTrackId = item.trackId;
            sliceQuery.trackId = curTrackId;
            TrackInfoManager::Instance().GetTrackInfo(curTrackId, trackInfo);
            simpleSliceMap.clear();
            std::vector<CompeteSliceDomain> sliceVec;
            dataEngine->QueryAllFlagSlice(sliceQuery, sliceVec);
            std::unordered_map<uint64_t, uint32_t> depthCache;
            sliceAnalyzerPtr->ComputeDepthInfoByTrackId(sliceQuery, depthCache);
            for (const auto &slice : sliceVec) {
                simpleSliceMap[slice.flagId] = depthCache[slice.id];
            }
        }
        std::unordered_map<uint64_t, uint32_t> depthCache;
        sliceAnalyzerPtr->ComputeDepthInfoByTrackId(sliceQuery, depthCache);
        item.depth = simpleSliceMap[item.flowId];
        item.pid = trackInfo.processId;
        item.tid = trackInfo.threadId;
    }
    flowAnalyzerPtr->SortByFlowIdAndTimestampASC(flowPointResult);
    flowAnalyzerPtr->ComputeUintFlows(flowPointResult, params.category, flowDetailList);
    ServerLog::Info("Query Simulation flow category events. size:", flowDetailList.size());
}

void RenderEngine::QueryThreadDetail(const ThreadDetailParams &requestParams, UnitThreadDetailBody &responseBody,
    uint64_t trackId)
{
    CompeteSliceDomain competeSliceDomain;
    SliceQuery sliceQuery;
    sliceQuery.trackId = trackId;
    sliceQuery.rankId = requestParams.rankId;
    sliceQuery.sliceId = requestParams.id;
    sliceQuery.metaType = Protocol::STR_TO_ENUM<PROCESS_TYPE>(requestParams.metaType).value();
    dataEngine->QuerySliceDetailInfo(sliceQuery, competeSliceDomain);
    responseBody.data.selfTime = 0;
    responseBody.data.args = competeSliceDomain.args;
    responseBody.data.title = competeSliceDomain.name;
    responseBody.data.duration = competeSliceDomain.endTime - competeSliceDomain.timestamp; // 保证 endTime >= timestamp
    responseBody.data.inputShapes = competeSliceDomain.sliceShape.inputShapes;
    responseBody.data.inputDataTypes = competeSliceDomain.sliceShape.inputDataTypes;
    responseBody.data.inputFormats = competeSliceDomain.sliceShape.inputFormats;
    responseBody.data.outputShapes = competeSliceDomain.sliceShape.outputShapes;
    responseBody.data.outputDataTypes = competeSliceDomain.sliceShape.outputDataTypes;
    responseBody.data.outputFormats = competeSliceDomain.sliceShape.outputFormats;
    SliceAnalyzer sliceAnalyzer;
    std::vector<SliceDomain> sliceVec;
    sliceAnalyzer.ComputeSliceDomainVecByTrackId(sliceQuery, sliceVec);
    SliceDomain target;
    target.id = competeSliceDomain.id;
    target.timestamp = competeSliceDomain.timestamp;
    auto it = std::lower_bound(sliceVec.begin(), sliceVec.end(), target, SliceDomain::CompareTimestampASC);
    if (it == sliceVec.end()) {
        return;
    }
    const uint32_t targetDepth = it->depth + 1;
    const uint64_t targetTimestamp = competeSliceDomain.timestamp;
    const uint64_t targetEndTime = competeSliceDomain.endTime;
    uint64_t nextDepthTime = 0;
    for (auto item = it; item != sliceVec.end(); ++item) {
        if (item->timestamp >= targetTimestamp && item->endTime <= targetEndTime && item->depth == targetDepth) {
            nextDepthTime += item->endTime - item->timestamp; // 从数据库查询得到。业务上保证 item->endTime >= item->timestamp
        }
    }
    if (nextDepthTime > 0 && nextDepthTime <= responseBody.data.duration) {
        responseBody.data.selfTime = responseBody.data.duration - nextDepthTime;
    }
}

CompeteSliceDomain RenderEngine::FindSliceByTimePoint(const std::string &fileId, const std::string &name,
    uint64_t timePoint, const std::string &metaType)
{
    SliceQuery sliceQuery;
    sliceQuery.rankId = fileId;
    sliceQuery.name = name;
    sliceQuery.metaType = Protocol::STR_TO_ENUM<PROCESS_TYPE>(metaType).value();
    sliceQuery.timePoint = timePoint;
    CompeteSliceDomain slice;
    bool res = dataEngine->QuerySliceByTimepointAndName(sliceQuery, slice);
    if (!res) {
        ServerLog::Warn("Failed to find slice, name is: %", name);
        return slice;
    }
    std::unordered_map<uint64_t, uint32_t> depthCache;
    std::unique_ptr<SliceAnalyzer> sliceAnalyzerPtr = std::make_unique<SliceAnalyzer>();
    sliceQuery.trackId = slice.trackId;
    sliceAnalyzerPtr->SetRepository(dataEngine);
    sliceAnalyzerPtr->ComputeDepthInfoByTrackId(sliceQuery, depthCache);
    slice.depth = depthCache[slice.id];
    return slice;
}
}
