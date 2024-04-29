/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "FlowAnalyzer.h"
namespace Dic::Module::Timeline {
FlowAnalyzer::FlowAnalyzer() = default;
std::vector<Protocol::FlowName> FlowAnalyzer::ComputeFlowBySliceVec(const std::vector<Protocol::FlowName> &flowNameVec,
    std::vector<Protocol::SimpleSlice> &sliceVec)
{
    std::vector<Protocol::FlowName> res;
    if (std::empty(sliceVec)) {
        return res;
    }
    Protocol::SimpleSlice currentSlice = sliceVec[0];
    // 移除当前选中的算子
    sliceVec.erase(sliceVec.begin());
    bool isEmplace = true;
    for (const auto &flowItem : flowNameVec) {
        for (const auto &simpleSliceItem : sliceVec) {
            // 开始节点和最高的算子对应
            if (flowItem.type == Protocol::LINE_START && flowItem.timestamp >= simpleSliceItem.timestamp &&
                flowItem.timestamp <= simpleSliceItem.endTime) {
                isEmplace = false;
                break;
            }
        }
        // 结束节点和算子开始时间对应
        if ((flowItem.type == Protocol::LINE_END || flowItem.type == Protocol::LINE_END_OPTIONAL) &&
            flowItem.timestamp != currentSlice.timestamp) {
            isEmplace = false;
        }
        if (isEmplace) {
            res.emplace_back(flowItem);
        }
        isEmplace = true;
    }
    return res;
}

void FlowAnalyzer::ComputeCategoryAndFlowMap(const std::vector<FlowDetailDto> &flowDetailVec,
    std::map<std::string, std::vector<Protocol::UnitSingleFlow>> &flowMap, uint64_t minTimestamp)
{
    const static int FLOW_COUNT = 2; // from + to
    if (flowDetailVec.size() != FLOW_COUNT) {
        return;
    }
    Protocol::UnitSingleFlow unitSingleFlow;
    unitSingleFlow.title = flowDetailVec[0].name;
    unitSingleFlow.cat = flowDetailVec[0].cat;
    unitSingleFlow.id = flowDetailVec[0].flowId;
    FlowDetailDto from(flowDetailVec[0]);
    FlowDetailDto to(flowDetailVec[1]);
    if (from.timestamp > to.timestamp) {
        from = flowDetailVec[1];
        to = flowDetailVec[0];
    }
    unitSingleFlow.from.id = from.id;
    unitSingleFlow.from.pid = from.pid;
    unitSingleFlow.from.tid = from.tid;
    unitSingleFlow.from.timestamp = from.timestamp - minTimestamp;
    unitSingleFlow.from.duration = from.duration;
    unitSingleFlow.from.depth = from.depth;
    unitSingleFlow.from.name = from.sliceName;
    unitSingleFlow.to.id = to.id;
    unitSingleFlow.to.pid = to.pid;
    unitSingleFlow.to.tid = to.tid;
    unitSingleFlow.to.timestamp = to.timestamp - minTimestamp;
    unitSingleFlow.to.duration = to.duration;
    unitSingleFlow.to.depth = to.depth;
    unitSingleFlow.to.name = to.sliceName;
    flowMap[unitSingleFlow.cat].emplace_back(unitSingleFlow);
}

void FlowAnalyzer::ComputeSingleFlowDetail(const std::vector<Protocol::SimpleSlice> &simpliceVec,
    FlowDetailDto &flowDetailDto)
{
    if (std::empty(simpliceVec)) {
        Server::ServerLog::Warn("SimpliceVec is Empty");
        return;
    }
    Protocol::SimpleSlice simpleSlice;
    // 连线开始点取最高，连线结束点取时间相等
    if (flowDetailDto.type == Protocol::LINE_START) {
        simpleSlice = simpliceVec.front();
    }
    if (flowDetailDto.type == Protocol::LINE_END || flowDetailDto.type == Protocol::LINE_END_OPTIONAL) {
        for (const auto &tempSlice : simpliceVec) {
            if (tempSlice.timestamp == flowDetailDto.flowTimestamp) {
                simpleSlice = tempSlice;
                break;
            }
        }
    }
    flowDetailDto.id = std::to_string(simpleSlice.id);
    flowDetailDto.depth = simpleSlice.depth;
    flowDetailDto.sliceName = simpleSlice.name;
    flowDetailDto.duration = simpleSlice.endTime - simpleSlice.timestamp;
    flowDetailDto.timestamp = simpleSlice.timestamp;
}
}
