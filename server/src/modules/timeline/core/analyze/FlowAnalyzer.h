/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */


#ifndef PROFILER_SERVER_FLOWANALYZER_H
#define PROFILER_SERVER_FLOWANALYZER_H
#include <vector>
#include <set>
#include <string>
#include <map>
#include "TimelineProtocolResponse.h"
#include "TraceDatabaseDef.h"
#include "ServerLog.h"
namespace Dic::Module::Timeline {
class VirtualFlowAnalyzer {
public:
    explicit VirtualFlowAnalyzer() = default;
    ~VirtualFlowAnalyzer() = default;
    virtual std::vector<Protocol::FlowName> ComputeFlowBySliceVec(const std::vector<Protocol::FlowName> &flowNameVec,
        std::vector<Protocol::SimpleSlice> &sliceVec) = 0;
    virtual void ComputeCategoryAndFlowMap(const std::vector<FlowDetailDto> &flowDetailVec,
        std::map<std::string, std::vector<Protocol::UnitSingleFlow>> &flowMap, uint64_t minTimestamp) = 0;
    virtual void ComputeSingleFlowDetail(const std::vector<Protocol::SimpleSlice> &simpliceVec,
        FlowDetailDto &flowDetailDto) = 0;
};

class FlowAnalyzer : public VirtualFlowAnalyzer {
public:
    explicit FlowAnalyzer();
    ~FlowAnalyzer() = default;
    std::vector<Protocol::FlowName> ComputeFlowBySliceVec(const std::vector<Protocol::FlowName> &flowNameVec,
        std::vector<Protocol::SimpleSlice> &sliceVec) override;
    void ComputeCategoryAndFlowMap(const std::vector<FlowDetailDto> &flowDetailVec,
        std::map<std::string, std::vector<Protocol::UnitSingleFlow>> &flowMap, uint64_t minTimestamp) override;
    void ComputeSingleFlowDetail(const std::vector<Protocol::SimpleSlice> &simpliceVec,
        FlowDetailDto &flowDetailDto) override;
};
}


#endif // PROFILER_SERVER_FLOWANALYZER_H
