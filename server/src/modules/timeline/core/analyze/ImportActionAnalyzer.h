/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_IMPORTACTIONANALYZER_H
#define PROFILER_SERVER_IMPORTACTIONANALYZER_H
#include <vector>
#include <list>
#include <set>
#include <unordered_map>
#include "TimelineProtocolResponse.h"
#include "SliceDepthCacheManager.h"

namespace Dic::Module::Timeline {
class VirtualImportActionAnalyzer {
public:
    explicit VirtualImportActionAnalyzer() = default;
    ~VirtualImportActionAnalyzer() = default;
    virtual void UpdateAllSimulationSliceDepthWithNoOverlap(std::vector<Protocol::SimpleSlice> &rowThreadTraceVec,
        uint64_t trackId) = 0;
};

class ImportActionAnalyzer : public VirtualImportActionAnalyzer {
public:
    explicit ImportActionAnalyzer();
    ~ImportActionAnalyzer() = default;
    void UpdateAllSimulationSliceDepthWithNoOverlap(std::vector<Protocol::SimpleSlice> &rowThreadTraceVec,
        uint64_t trackId) override;
private:
    static void ComputeSimulationSliceDepth(std::vector<Protocol::SimpleSlice> &rowThreadTraceVec,
        SliceDepthCacheStruct &sliceDepthCacheStruct);

    static void ComputeSingleSliceDepth(Protocol::SimpleSlice &rowThreadTrace, std::set<int32_t> &depthSet);
};
}
#endif // PROFILER_SERVER_IMPORTACTIONANALYZER_H
