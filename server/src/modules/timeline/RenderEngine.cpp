//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
//
#include "pch.h"
#include "DomainObject.h"
#include "ProtocolEnumUtil.h"
#include "SliceAnalyzer.h"
#include "FlowAnalyzer.h"
#include "TrackInfoManager.h"
#include "RenderEngine.h"
namespace Dic::Module::Timeline {
using namespace Dic::Server;
using namespace Dic::Protocol;
void RenderEngine::SetDataEngineInterface(std::shared_ptr<DataEngineInterface> dataEngineInterface)
{
    dataEngine = dataEngineInterface;
}

void RenderEngine::QueryThreadTraces(const Protocol::UnitThreadTracesParams &requestParams,
    Protocol::UnitThreadTracesBody &responseBody, uint64_t minTimestamp, int64_t traceId)
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
        bool isHide = requestParams.isHideFlagEvents && (item.name == "SET_FLAG" || item.name == "WAIT_FLAG");
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
        item.depth = sliceAnalyzerPtr->ComputeFlowPointDepth(cacheSlices, item.type, item.timestamp + minTimestamp);
        item.pid = trackInfo.processId;
        item.tid = trackInfo.threadId;
    }
    flowAnalyzerPtr->SortByFlowIdAndTimestampASC(flowPointResult);
    flowAnalyzerPtr->ComputeUintFlows(flowPointResult, params.category, flowDetailList);
    ServerLog::Info("Query flow category events. size:", flowDetailList.size());
    return true;
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
}
