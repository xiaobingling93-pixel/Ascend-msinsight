/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SLICEANALYZER_H
#define PROFILER_SERVER_SLICEANALYZER_H
#include <set>
#include "TimelineProtocolResponse.h"
#include "SliceDepthCacheManager.h"
#include "DominQuery.h"
#include "DomainObject.h"
#include "Repository.h"
#include "CacheManager.h"
namespace Dic::Module::Timeline {
class SliceAnalyzer {
public:
    SliceAnalyzer();
    ~SliceAnalyzer();
    static void SortByTimestampASC(std::vector<SliceDomain> &cacheSlices);
    static uint32_t ComputeFlowPointDepth(std::vector<SliceDomain> &cacheSlices, std::string &type, uint64_t timestamp);
    static void CalculateSelfTime(std::vector<Protocol::SimpleSlice> &rows,
        std::map<std::string, uint64_t> &selfTimeKeyValue, uint64_t startTime, uint64_t endTime);
    void ComputeScreenSliceIds(const SliceQuery &sliceQuery, std::set<uint64_t> &ids, uint64_t &maxDepth,
        bool &havePythonFunction, std::map<uint64_t, int32_t> &depthMap);

private:
    std::unique_ptr<Repository> repository;
    static bool CompareTimestampASC(const SliceDomain &first, const SliceDomain &second);
    static void AddData(std::map<std::string, uint64_t> &selfTimeKeyValue, const std::string &name,
        uint64_t tmpSelfTime);
    static void ComputeSmallScreenSliceIds(uint64_t startTime, uint64_t endTime,
        const std::vector<SliceDomain> &cacheSlices, std::set<uint64_t> &ids);
    static void ComputeDepthResultIds(uint64_t startTime, uint64_t endTime,
        const std::vector<SliceDomain> &slicesDomainVec, uint64_t unitTime, std::set<uint64_t> &ids);
    static void ComputeDepth(std::vector<SliceDomain> &sliceDomainVec, const std::set<uint64_t> &pythonFunctionIds,
        std::vector<std::vector<SliceDomain>> &depthCacheSlice, std::map<uint64_t, int32_t> &depthMap);
    static std::set<uint64_t> ComputeResultIds(uint64_t startTime, uint64_t endTime,
        std::vector<std::vector<SliceDomain>> &depthSlicesDomainVec);
};
}
#endif // PROFILER_SERVER_SLICEANALYZER_H
