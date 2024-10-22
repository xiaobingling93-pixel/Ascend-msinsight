/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "TraceDatabaseDef.h"
#include "DomainObject.h"
#include "DominQuery.h"
#include "SliceCacheManager.h"
#include "TextRepository.h"
#include "FlowAnalyzer.h"

using namespace Dic::Server;
namespace Dic::Module::Timeline {
FlowAnalyzer::FlowAnalyzer()
{
    if (repository == nullptr) {
        repository = std::make_unique<TextRepository>();
    }
}

void FlowAnalyzer::SetRepository(std::unique_ptr<FlowRepoInterface> repositoryDependency)
{
    repository = std::move(repositoryDependency);
}

std::vector<FlowPoint> FlowAnalyzer::ComputeAllFlowPointBySliceId(FlowQuery &flowQuery, const std::string &sliceId)
{
    std::unordered_set<std::string> onSliceFlowPointSet = ComputeOnSliceFlowPointBySliceId(flowQuery, sliceId);
    std::vector<FlowPoint> res;
    if (std::empty(onSliceFlowPointSet)) {
        return res;
    }
    for (const auto &item : onSliceFlowPointSet) {
        flowQuery.flowId = item;
        repository->QueryFlowPointByFlowId(flowQuery, res);
    }
    return res;
}

std::unordered_set<std::string> FlowAnalyzer::ComputeOnSliceFlowPointBySliceId(const FlowQuery &flowQuery,
    const std::string &sliceId)
{
    std::unordered_set<std::string> res;
    std::vector<FlowPoint> flowPointVec;
    repository->QueryFlowPointByTimeRange(flowQuery, flowPointVec);
    ServerLog::Info("flowPointVec is: ", flowPointVec.size());
    auto &instance = SliceCacheManager::Instance();
    std::vector<SliceDomain> sliceVec = instance.GetSliceDomainVec(std::to_string(flowQuery.trackId));
    // 此时是前端打开泳道点击算子，缓存中必定存在泳道下所有的算子数据,若不存在说明是异常情况
    if (std::empty(sliceVec)) {
        return res;
    }
    for (const auto &item : flowPointVec) {
        auto it = ComputeSliceByFlowPoint(item, sliceVec);
        if (it != sliceVec.end() && std::to_string(it->id) == sliceId) {
            res.emplace(item.flowId);
        }
    }
    return res;
}

std::vector<SliceDomain>::const_iterator FlowAnalyzer::ComputeSliceByFlowPoint(const FlowPoint &flowPoint,
    const std::vector<SliceDomain> &sliceVec) const
{
    SliceDomain slice;
    slice.timestamp = flowPoint.timestamp;
    slice.id = 0;
    auto it = sliceVec.begin();
    if (flowPoint.type == Protocol::LINE_START) {
        it = std::lower_bound(sliceVec.begin(), sliceVec.end(), slice, SliceDomain::CompareTimestampASC);
        if (it != sliceVec.end() && it->timestamp == flowPoint.timestamp) {
            return it;
        }

        while (it != sliceVec.end() && it > sliceVec.begin()) {
            it--;
            if (it->timestamp <= flowPoint.timestamp && it->endTime >= flowPoint.timestamp) {
                break;
            }
        }
        return it;
    }
    if (flowPoint.type == Protocol::LINE_END || flowPoint.type == Protocol::LINE_END_OPTIONAL) {
        it = std::lower_bound(sliceVec.begin(), sliceVec.end(), slice, SliceDomain::CompareTimestampASC);
        if (it != sliceVec.end()) {
            return it;
        }
    }
    return it;
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
    if (from.type != to.type && to.type == Protocol::LINE_START) {
        from = flowDetailVec[1];
        to = flowDetailVec[0];
    }
    unitSingleFlow.from.id = from.id;
    unitSingleFlow.from.pid = from.pid;
    unitSingleFlow.from.tid = from.tid;
    if (from.timestamp < minTimestamp || to.timestamp < minTimestamp) {
        return;
    }
    unitSingleFlow.from.timestamp = from.timestamp - minTimestamp;
    unitSingleFlow.from.depth = from.depth;
    unitSingleFlow.to.id = to.id;
    unitSingleFlow.to.pid = to.pid;
    unitSingleFlow.to.tid = to.tid;
    unitSingleFlow.to.timestamp = to.timestamp - minTimestamp;
    unitSingleFlow.to.depth = to.depth;
    flowMap[unitSingleFlow.cat].emplace_back(unitSingleFlow);
}

void FlowAnalyzer::SortByTrackIdASC(std::vector<FlowPoint> &flowCategoryEventsDtoVec)
{
    std::sort(flowCategoryEventsDtoVec.begin(), flowCategoryEventsDtoVec.end(), CompareTrackIdASC);
}

void FlowAnalyzer::SortByFlowIdAndTimestampASC(std::vector<FlowPoint> &flowCategoryEventsDtoVec)
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
void FlowAnalyzer::ComputeScreenFlowPoint(const std::vector<FlowPoint> &flowEventsVec, uint64_t startTime,
    uint64_t endTime, std::vector<FlowPoint> &flowIdResult)
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

/**
 * 根据连线点组装连线
 * @param flowEventsVec 连线点
 * @param category 连线类别
 * @param flowDetailList 结果集
 */
void FlowAnalyzer::ComputeUintFlows(const std::vector<FlowPoint> &flowEventsVec,
    const std::string &category, std::vector<std::unique_ptr<Protocol::UnitSingleFlow>> &flowDetailList)
{
    std::string curFlowId;
    Protocol::FlowLocation location;
    Protocol::FlowLocation *locationPtr = &location;
    for (const auto &flow : flowEventsVec) {
        std::string type = flow.type;
        std::string flowId = flow.flowId;
        if (type == Protocol::LINE_START || flowId != curFlowId) {
            location.pid = flow.pid;
            location.tid = flow.tid;
            location.depth = flow.depth;
            location.timestamp = flow.timestamp;
            location.type = type;
            location.rankId = flow.rankId;
            locationPtr = &location;
        } else if ((type == Protocol::LINE_END || type == Protocol::LINE_END_OPTIONAL) && flowId == curFlowId) {
            auto flowEvent = std::make_unique<Protocol::UnitSingleFlow>();
            flowEvent->cat = category;
            flowEvent->from = *locationPtr;
            flowEvent->to.pid = flow.pid;
            flowEvent->to.tid = flow.tid;
            flowEvent->to.depth = flow.depth;
            flowEvent->to.timestamp = flow.timestamp;
            flowEvent->to.rankId = flow.rankId;
            locationPtr = &(flowEvent->to);
            if (flowEvent->from.type == Protocol::LINE_START) {
                flowDetailList.emplace_back(std::move(flowEvent));
            }
        }
        curFlowId = flowId;
    }
}

void FlowAnalyzer::OfferFlowPointPair(const std::vector<FlowPoint> &flowEventsVec,
    std::vector<FlowPoint> &flowIdResult, FlowPointSampleStruct &flowPointSampleStruct,
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

void FlowAnalyzer::GroupSampleFlowPoint(const std::vector<FlowPoint> &flowEventsVec, uint64_t startTime,
    uint64_t endTime, FlowPointSampleStruct &flowPointSampleStruct)
{
    uint64_t curTrackId = std::numeric_limits<uint64_t>::max();
    uint64_t index = 0;
    // 此处500是把屏幕平均分成500份，以一份屏幕的宽度为采集连线点的最小步长
    uint64_t uintTime = (endTime - startTime) / 500;
    for (const auto &item : flowEventsVec) {
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
        index++;
        if (uintTime == 0 && item.timestamp < startTime) {
            continue;
        }
        if (uintTime == 0 && item.timestamp > endTime) {
            continue;
        }
        ComputePointOnScreen(flowPointSampleStruct, uintTime, item);
    }
}

void FlowAnalyzer::ComputePointOnScreen(FlowPointSampleStruct &flowPointSampleStruct, uint64_t uintTime,
    const FlowPoint &flowPoint)
{
    if (uintTime == 0 && flowPoint.type == Protocol::LINE_START) {
        flowPointSampleStruct.startPointResultSet.emplace(flowPoint.flowId);
        return;
    }
    if (uintTime == 0 && (flowPoint.type == Protocol::LINE_END || flowPoint.type == Protocol::LINE_END_OPTIONAL)) {
        flowPointSampleStruct.endPointResultSet.emplace(flowPoint.flowId);
        return;
    }
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


bool FlowAnalyzer::CompareTrackIdASC(const FlowPoint &first, const FlowPoint &second)
{
    if (first.trackId < second.trackId) {
        return true;
    }
    if (first.trackId == second.trackId && first.id < second.id) {
        return true;
    }
    return false;
}

bool FlowAnalyzer::CompareFlowIdAndTimestampASC(const FlowPoint &first, const FlowPoint &second)
{
    if (first.flowId < second.flowId) {
        return true;
    }
    if (first.flowId > second.flowId) {
        return false;
    }
    if (second.type != Protocol::LINE_START && first.type == Protocol::LINE_START) {
        return true;
    }
    if (first.type != Protocol::LINE_START && second.type == Protocol::LINE_START) {
        return false;
    }
    if (first.type != Protocol::LINE_START && second.type != Protocol::LINE_START && first.id < second.id) {
        return true;
    }
    if (first.type == Protocol::LINE_START && second.type == Protocol::LINE_START && first.id < second.id) {
        return true;
    }
    return false;
}
}
