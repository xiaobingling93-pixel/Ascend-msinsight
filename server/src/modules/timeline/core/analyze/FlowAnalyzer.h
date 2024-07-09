/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */


#ifndef PROFILER_SERVER_FLOWANALYZER_H
#define PROFILER_SERVER_FLOWANALYZER_H
#include <vector>
#include <set>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include "TimelineProtocolResponse.h"
#include "TraceDatabaseDef.h"
#include "ServerLog.h"
#include "SliceCacheManager.h"
namespace Dic::Module::Timeline {
struct FlowPointSampleStruct {
    // 连线去重集合
    std::unordered_set<std::string> resultFlowIdSet;
    // 开始点集合
    std::unordered_map<std::string, uint64_t> startPointMap;
    // 结束点集合
    std::unordered_map<std::string, uint64_t> endPointMap;
    // 开始点的采样集合
    std::unordered_set<std::string> startPointResultSet;
    // 结束点的采样集合
    std::unordered_set<std::string> endPointResultSet;
    // 开始点所在区间最大时间
    uint64_t curBeginLimitTime = 0;
    // 开始点所在区间最小时间
    uint64_t curBeginStartTime = 0;
    // 结束点所在区间最大时间
    uint64_t curEndLimitTime = 0;
    // 结束点所在区间最小时间
    uint64_t curEndStartTime = 0;
};

class FlowAnalyzer {
public:
    explicit FlowAnalyzer();
    ~FlowAnalyzer() = default;
    std::vector<Protocol::FlowName> ComputeFlowBySliceVec(const std::vector<Protocol::FlowName> &flowNameVec,
        std::vector<Protocol::SimpleSlice> &sliceVec);
    void ComputeCategoryAndFlowMap(const std::vector<FlowDetailDto> &flowDetailVec,
        std::map<std::string, std::vector<Protocol::UnitSingleFlow>> &flowMap, uint64_t minTimestamp);
    void ComputeSingleFlowDetail(const std::vector<Protocol::SimpleSlice> &simpliceVec,
        FlowDetailDto &flowDetailDto);
    void SortByTrackIdASC(std::vector<FlowCategoryEventsDto> &FlowCategoryEventsDtoVec);
    void SortByFlowIdAndTimestampASC(std::vector<FlowCategoryEventsDto> &flowCategoryEventsDtoVec);
    void ComputeScreenFlowPoint(const std::vector<FlowCategoryEventsDto> &flowEventsVec, uint64_t startTime,
        uint64_t endTime, std::vector<FlowCategoryEventsDto> &flowIdResult);
    void ComputeUintFlows(const std::vector<FlowCategoryEventsDto> &flowEventsVec, const std::string &category,
        std::vector<std::unique_ptr<Protocol::UnitSingleFlow>> &flowDetailList);

private:
    static bool CompareTrackIdASC(const FlowCategoryEventsDto &first, const FlowCategoryEventsDto &second);
    static bool CompareFlowIdAndTimestampASC(const FlowCategoryEventsDto &first, const FlowCategoryEventsDto &second);
    static void GroupSampleFlowPoint(const std::vector<FlowCategoryEventsDto> &flowEventsVec, uint64_t startTime,
        uint64_t endTime, FlowPointSampleStruct &flowPointSampleStruct);
    static void ComputePointOnScreen(FlowPointSampleStruct &flowPointSampleStruct, uint64_t uintTime,
        const FlowCategoryEventsDto &flowPoint);
    void OfferFlowPointPair(const std::vector<FlowCategoryEventsDto> &flowEventsVec,
        std::vector<FlowCategoryEventsDto> &flowIdResult, FlowPointSampleStruct &flowPointSampleStruct,
        const std::string &flowId) const;
};
}


#endif // PROFILER_SERVER_FLOWANALYZER_H
