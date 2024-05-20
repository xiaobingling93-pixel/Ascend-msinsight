/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "algorithm"
#include "TraceDatabaseDef.h"
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

void FlowAnalyzer::SortByTrackIdASC(std::vector<FlowCategoryEventsDto> &flowCategoryEventsDtoVec)
{
    std::sort(flowCategoryEventsDtoVec.begin(), flowCategoryEventsDtoVec.end(), CompareTrackIdASC);
}

void FlowAnalyzer::SortByFlowIdAndTimestampASC(std::vector<FlowCategoryEventsDto> &flowCategoryEventsDtoVec)
{
    std::sort(flowCategoryEventsDtoVec.begin(), flowCategoryEventsDtoVec.end(), CompareFlowIdAndTimestampASC);
}

/**
 * 计算屏幕上需要呈现的连线点
 * @param flowEventsVec 数据集
 * @param startTime 屏幕开始时间
 * @param endTime 屏幕结束时间
 * @param flowIdResult 计算结果
 */
void FlowAnalyzer::ComputeScreenFlowPoint(const std::vector<FlowCategoryEventsDto> &flowEventsVec, uint64_t startTime,
    uint64_t endTime, std::vector<FlowCategoryEventsDto> &flowIdResult)
{
    FlowPointSampleStruct flowPointSampleStruct;
    GroupSampleFlowPoint(flowEventsVec, startTime, endTime, flowPointSampleStruct);
    for (const auto &item : flowPointSampleStruct.startPointResultSet) {
        OfferFlowPointPair(flowEventsVec, flowIdResult, flowPointSampleStruct, item);
    }
    for (const auto &item : flowPointSampleStruct.endPointResultSet) {
        OfferFlowPointPair(flowEventsVec, flowIdResult, flowPointSampleStruct, item);
    }
}

void FlowAnalyzer::OfferFlowPointPair(const std::vector<FlowCategoryEventsDto> &flowEventsVec,
    std::vector<FlowCategoryEventsDto> &flowIdResult, FlowPointSampleStruct &flowPointSampleStruct,
    const std::string &flowId) const
{
    // 过滤重复连线
    if (flowPointSampleStruct.resultFlowIdSet.count(flowId) > 0) {
        return;
    }
    // 过滤屏幕外右边的连线
    if (flowPointSampleStruct.endPointMap.count(flowId) == 0) {
        return;
    }
    // 过滤屏幕外左边的连线
    if (flowPointSampleStruct.startPointMap.count(flowId) == 0) {
        return;
    }
    // 收集在屏幕中间可展示的连线
    flowIdResult.emplace_back(flowEventsVec[flowPointSampleStruct.endPointMap[flowId]]);
    flowIdResult.emplace_back(flowEventsVec[flowPointSampleStruct.startPointMap[flowId]]);
    flowPointSampleStruct.resultFlowIdSet.emplace(flowId);
}

void FlowAnalyzer::GroupSampleFlowPoint(const std::vector<FlowCategoryEventsDto> &flowEventsVec, uint64_t startTime,
    uint64_t endTime, FlowPointSampleStruct &flowPointSampleStruct)
{
    int64_t curTrackId = -1;
    int64_t index = -1;
    // 此处500是把屏幕平均分成500份，以一份屏幕的宽度为采集连线点的最小步长
    uint64_t uintTime = (endTime - startTime) / 500;
    if (uintTime < 1) {
        uintTime = 1;
    }
    for (const auto &item : flowEventsVec) {
        index++;
        if (curTrackId != item.trackId) {
            curTrackId = item.trackId;
            flowPointSampleStruct.curBeginLimitTime = uintTime;
            flowPointSampleStruct.curBeginStartTime = 0;
            flowPointSampleStruct.curEndLimitTime = uintTime;
            flowPointSampleStruct.curEndStartTime = 0;
        }
        // 收集全部开始点
        if (item.timestamp <= endTime && item.type == Protocol::LINE_START) {
            flowPointSampleStruct.startPointMap[item.flowId] = index;
        }
        // 收集全部结束点
        if (item.timestamp >= startTime &&
            (item.type == Protocol::LINE_END || item.type == Protocol::LINE_END_OPTIONAL)) {
            flowPointSampleStruct.endPointMap[item.flowId] = index;
        }
        ComputePointOnScreen(flowPointSampleStruct, uintTime, item);
    }
}

void FlowAnalyzer::ComputePointOnScreen(FlowPointSampleStruct &flowPointSampleStruct, uint64_t uintTime,
    const FlowCategoryEventsDto &flowPoint)
{
    // 计算可能需要展示在屏幕上的开始点
    if (flowPoint.type == Protocol::LINE_START) {
        while (flowPoint.timestamp >= flowPointSampleStruct.curBeginLimitTime) {
            flowPointSampleStruct.curBeginLimitTime += uintTime;
            flowPointSampleStruct.curBeginStartTime += uintTime;
        }
        if (flowPoint.timestamp >= flowPointSampleStruct.curBeginStartTime) {
            flowPointSampleStruct.startPointResultSet.emplace(flowPoint.flowId);
            flowPointSampleStruct.curBeginLimitTime += uintTime;
            flowPointSampleStruct.curBeginStartTime += uintTime;
        }
    }
    // 计算可能需要展示在屏幕上的结束点
    if ((flowPoint.type == Protocol::LINE_END || flowPoint.type == Protocol::LINE_END_OPTIONAL)) {
        while (flowPoint.timestamp >= flowPointSampleStruct.curEndLimitTime) {
            flowPointSampleStruct.curEndStartTime += uintTime;
            flowPointSampleStruct.curEndLimitTime += uintTime;
        }
        if (flowPoint.timestamp >= flowPointSampleStruct.curEndStartTime) {
            flowPointSampleStruct.endPointResultSet.emplace(flowPoint.flowId);
            flowPointSampleStruct.curEndStartTime += uintTime;
            flowPointSampleStruct.curEndLimitTime += uintTime;
        }
    }
}


bool FlowAnalyzer::CompareTrackIdASC(const FlowCategoryEventsDto &first, const FlowCategoryEventsDto &second)
{
    if (first.trackId < second.trackId) {
        return true;
    }
    if (first.trackId == second.trackId && first.id < second.id) {
        return true;
    }
    return false;
}

bool FlowAnalyzer::CompareFlowIdAndTimestampASC(const FlowCategoryEventsDto &first, const FlowCategoryEventsDto &second)
{
    if (first.flowId < second.flowId) {
        return true;
    }
    if (first.flowId == second.flowId && first.timestamp < second.timestamp) {
        return true;
    }
    if (first.timestamp == second.timestamp && first.id < second.id) {
        return true;
    }
    return false;
}
}
