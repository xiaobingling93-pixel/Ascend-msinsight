/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "ImportActionAnalyzer.h"
namespace Dic::Module::Timeline {
ImportActionAnalyzer::ImportActionAnalyzer() = default;

void ImportActionAnalyzer::UpdateAllSimulationSliceDepthWithNoOverlap(
    std::vector<Protocol::SimpleSlice> &rowThreadTraceVec, const uint64_t trackId)
{
    std::list<Protocol::SimpleSlice> sliceDepthHelper;
    ComputeSimulationSliceDepth(rowThreadTraceVec, sliceDepthHelper);
    std::unordered_map<uint64_t, int32_t> sliceIdAndDepthMap;
    for (const auto &item : sliceDepthHelper) {
        sliceIdAndDepthMap[item.id] = item.depth;
    }
    SliceDepthCacheManager::Instance().PutSliceDepthCacheStructByFileIdAndTrackId(trackId, sliceIdAndDepthMap);
}

void ImportActionAnalyzer::ComputeSimulationSliceDepth(std::vector<Protocol::SimpleSlice> &rowThreadTraceVec,
    std::list<Protocol::SimpleSlice> &sliceDepthHelper)
{
    sliceDepthHelper.clear();
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
        }
        for (const auto &traceVec : sliceDepthHelper) {
            if (rowThreadTrace.endTime >= traceVec.endTime) {
                break;
            }
            index++;
        }
        if (depthSet.empty()) {
            rowThreadTrace.depth = 0;
            std::advance(iterator, index);
            sliceDepthHelper.insert(iterator, rowThreadTrace);
            continue;
        }
        int32_t maxDepth = *(--depthSet.end());
        bool isBlank = false;
        for (int i = maxDepth; i >= 0; --i) {
            if (depthSet.find(i) == depthSet.end()) {
                isBlank = true;
                rowThreadTrace.depth = i;
            }
        }
        if (!isBlank) {
            rowThreadTrace.depth = maxDepth + 1;
        }
        std::advance(iterator, index);
        sliceDepthHelper.insert(iterator, rowThreadTrace);
    }
}
}