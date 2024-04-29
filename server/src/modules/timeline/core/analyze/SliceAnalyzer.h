/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SLICEANALYZER_H
#define PROFILER_SERVER_SLICEANALYZER_H
#include <set>
#include "TimelineProtocolResponse.h"
#include "SliceDepthCacheManager.h"
#include "CacheManager.h"
namespace Dic::Module::Timeline {
class VirtualSliceAnalyzer {
public:
    explicit VirtualSliceAnalyzer() = default;
    ~VirtualSliceAnalyzer() = default;
    virtual std::set<int64_t> ComputeResultIds(uint64_t startTime, uint64_t endtTime, uint64_t minTimestamp,
        std::vector<CacheSlice> &cacheSlices) = 0;
};

class SliceAnalyzer : public VirtualSliceAnalyzer {
public:
    explicit SliceAnalyzer();
    ~SliceAnalyzer() = default;
    std::set<int64_t> ComputeResultIds(uint64_t startTime, uint64_t endtTime, uint64_t minTimestamp,
        std::vector<CacheSlice> &cacheSlices) override;
};
}
#endif // PROFILER_SERVER_SLICEANALYZER_H
