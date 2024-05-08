/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "ImportActionAnalyzer.h"
namespace Dic::Module::Timeline {
ImportActionAnalyzer::ImportActionAnalyzer() = default;

void ImportActionAnalyzer::UpdateAllSimulationSliceDepthWithNoOverlap(
    std::vector<Protocol::SimpleSlice> &rowThreadTraceVec, const uint64_t trackId)
{
    SliceDepthCacheStruct sliceDepthCacheStruct;
    ComputeSimulationSliceDepth(rowThreadTraceVec, sliceDepthCacheStruct);
    sliceDepthCacheStruct.trackId = trackId;
    SliceDepthCacheManager::Instance().PutSliceDepthCacheStructByFileIdAndTrackId(trackId, sliceDepthCacheStruct);
}

void ImportActionAnalyzer::ComputeSimulationSliceDepth(std::vector<Protocol::SimpleSlice> &rowThreadTraceVec,
    SliceDepthCacheStruct &sliceDepthCacheStruct)
{
    std::list<Protocol::SimpleSlice> sliceDepthHelper;
    int32_t trackMaxDepth = 0;
    std::unordered_map<uint64_t, int32_t> sliceIdAndDepthMap;
    for (auto &rowThreadTrace : rowThreadTraceVec) {
        std::set<int32_t> depthSet;
        uint64_t index = 0;
        auto iterator = sliceDepthHelper.begin();
        for (const auto &traceVec : sliceDepthHelper) {
            if (rowThreadTrace.timestamp <= traceVec.endTime) {
                depthSet.emplace(traceVec.depth);
            }
            if (rowThreadTrace.timestamp > traceVec.endTime) {
                break;
            }
            if (rowThreadTrace.endTime < traceVec.endTime) {
                ++index;
            }
        }
        if (depthSet.empty()) {
            rowThreadTrace.depth = 0;
            sliceIdAndDepthMap[rowThreadTrace.id] = 0;
            std::advance(iterator, index);
            sliceDepthHelper.insert(iterator, rowThreadTrace);
            continue;
        }
        ComputeSingleSliceDepth(rowThreadTrace, depthSet);
        trackMaxDepth = std::max(trackMaxDepth, rowThreadTrace.depth);
        sliceIdAndDepthMap[rowThreadTrace.id] = rowThreadTrace.depth;
        std::advance(iterator, index);
        sliceDepthHelper.insert(iterator, rowThreadTrace);
    }
    sliceDepthCacheStruct.sliceIdAndDepthMap = sliceIdAndDepthMap;
    sliceDepthCacheStruct.maxDepTh = trackMaxDepth;
}

void ImportActionAnalyzer::ComputeSingleSliceDepth(Protocol::SimpleSlice &rowThreadTrace, std::set<int32_t> &depthSet)
{
    uint32_t maxDepth = *(--depthSet.end());
    uint32_t depthSize = depthSet.size();
    bool isBlank = false;
    // 判断是否有空位
    if (maxDepth + 1 != depthSize) {
        isBlank = true;
        for (int i = maxDepth; i >= 0; --i) {
            if (depthSet.find(i) == depthSet.end()) {
                rowThreadTrace.depth = i;
            }
        }
    }
    if (!isBlank) {
        rowThreadTrace.depth = maxDepth + 1;
    }
}
}