// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#include "SliceTable.h"
#include "FlowTable.h"
#include "ThreadTable.h"
#include "TrackInfoManager.h"
#include "TextRepository.h"
namespace Dic::Module::Timeline {
void TextRepository::QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
    std::vector<SliceDomain> &sliceVec)
{
    TrackInfo trackInfo;
    auto &instance = TrackInfoManager::Instance();
    const bool isSuccess = instance.GetTrackInfo(sliceQuery.trackId, trackInfo);
    if (!isSuccess) {
        return;
    }
    SliceTable sliceTable;
    std::vector<SlicePO> tempSlicePOVec;
    sliceTable.Select(SliceColumn::ID, SliceColumn::TIMESTAMP, SliceColumn::ENDTIME)
        .Eq(SliceColumn::TRACKID, sliceQuery.trackId)
        .OrderBy(SliceColumn::TIMESTAMP, TableOrder::ASC)
        .OrderBy(SliceColumn::ID, TableOrder::ASC)
        .ExcuteQuery(trackInfo.cardId, tempSlicePOVec);
    for (const auto &item : tempSlicePOVec) {
        SliceDomain cachelice;
        cachelice.id = item.id;
        cachelice.timestamp = item.timestamp;
        cachelice.endTime = item.endTime;
        sliceVec.emplace_back(cachelice);
    }
}

void TextRepository::QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::vector<uint64_t> &sliceIds)
{
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(sliceQuery.trackId, trackInfo);
    if (!isSuccess) {
        return;
    }
    SliceTable sliceTable;
    std::vector<SlicePO> slicePOVec;
    sliceTable.Select(SliceColumn::ID)
        .Eq(SliceColumn::TRACKID, sliceQuery.trackId)
        .Eq(SliceColumn::CAT, sliceQuery.cat)
        .OrderBy(SliceColumn::ID, TableOrder::ASC)
        .ExcuteQuery(trackInfo.cardId, slicePOVec);
    for (const auto &item : slicePOVec) {
        sliceIds.emplace_back(item.id);
    }
}

uint64_t TextRepository::QueryPythonFunctionCountByTrackId(const SliceQuery &sliceQuery)
{
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(sliceQuery.trackId, trackInfo);
    if (!isSuccess) {
        return 0;
    }
    SliceTable sliceTable;
    uint64_t count = sliceTable.Eq(SliceColumn::TRACKID, sliceQuery.trackId)
                         .Eq(SliceColumn::CAT, sliceQuery.cat)
                         .Count(trackInfo.cardId);
    return count;
}

void TextRepository::QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
    std::vector<CompeteSliceDomain> &sliceVec)
{
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(sliceQuery.trackId, trackInfo);
    if (!isSuccess) {
        return;
    }
    SliceTable sliceTable;
    std::vector<SlicePO> slicePOVec;
    sliceTable.Select(SliceColumn::ID, SliceColumn::TIMESTAMP)
        .Select(SliceColumn::DURATION, SliceColumn::ENDTIME, SliceColumn::NAME)
        .Eq(SliceColumn::TRACKID, sliceQuery.trackId)
        .LessEq(SliceColumn::TIMESTAMP, sliceQuery.endTime + sliceQuery.minTimestamp)
        .Greater(SliceColumn::ENDTIME, sliceQuery.startTime + sliceQuery.minTimestamp)
        .ExcuteQuery(trackInfo.cardId, slicePOVec);
    for (const auto &item : slicePOVec) {
        CompeteSliceDomain temp;
        temp.id = item.id;
        temp.timestamp = item.timestamp;
        temp.duration = item.duration;
        temp.endTime = item.endTime;
        temp.name = item.name;
        sliceVec.emplace_back(std::move(temp));
    }
}

void TextRepository::QueryFlowPointByTimeRange(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec)
{
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(flowQuery.trackId, trackInfo);
    if (!isSuccess) {
        return;
    }
    FlowTable flowTable;
    std::vector<FlowPO> flowPOVec;
    flowTable.Select(FlowColumn::TYPE, FlowColumn::TIMESTAMP, FlowColumn::FLOW_ID)
        .GreaterEq(FlowColumn::TIMESTAMP, flowQuery.startTime + flowQuery.minTimestamp)
        .LessEq(FlowColumn::TIMESTAMP, flowQuery.endTime + flowQuery.minTimestamp)
        .Eq(FlowColumn::TRACK_ID, flowQuery.trackId)
        .GroupBy(FlowColumn::FLOW_ID)
        .ExcuteQuery(trackInfo.cardId, flowPOVec);
    for (const auto &item : flowPOVec) {
        FlowPoint flowPoint;
        flowPoint.timestamp = item.timestamp;
        flowPoint.type = item.type;
        flowPoint.flowId = item.flowId;
        flowPointVec.emplace_back(std::move(flowPoint));
    }
}

void TextRepository::QueryFlowPointByFlowId(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec)
{
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(flowQuery.trackId, trackInfo);
    if (!isSuccess) {
        return;
    }
    FlowTable flowTable;
    std::vector<FlowPO> flowPOVec;
    flowTable.Select(FlowColumn::NAME, FlowColumn::CAT, FlowColumn::FLOW_ID)
        .Select(FlowColumn::TIMESTAMP, FlowColumn::TYPE, FlowColumn::TRACK_ID)
        .Eq(FlowColumn::FLOW_ID, flowQuery.flowId)
        .ExcuteQuery(trackInfo.cardId, flowPOVec);
    for (const auto &item : flowPOVec) {
        FlowPoint flowPoint;
        flowPoint.name = item.name;
        flowPoint.cat = item.cat;
        flowPoint.flowId = item.flowId;
        flowPoint.timestamp = item.timestamp;
        flowPoint.type = item.type;
        flowPoint.trackId = item.trackId;
        flowPointVec.emplace_back(std::move(flowPoint));
    }
}

void TextRepository::QueryAllThreadInfo(const ThreadQuery &threadQuery,
    std::unordered_map<uint64_t, std::pair<std::string, std::string>> &threadInfo)
{
    ThreadTable threadTable;
    std::vector<ThreadPO> threadPOVec;
    threadTable.Select(ThreadColumn::TRACK_ID, ThreadColumn::PID, ThreadColumn::TID)
        .ExcuteQuery(threadQuery.fileId, threadPOVec);
    for (const auto &item : threadPOVec) {
        threadInfo[item.trackId] = {item.pid, item.tid};
    }
}

void TextRepository::QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
    std::vector<CompeteSliceDomain> &competeSliceVec)
{
    if (std::empty(sliceIds)) {
        return;
    }
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(sliceQuery.trackId, trackInfo);
    if (!isSuccess) {
        return;
    }
    SliceTable sliceTable;
    std::vector<SlicePO> tempSlicePOVec;
    sliceTable.Select(SliceColumn::ID, SliceColumn::TIMESTAMP, SliceColumn::ENDTIME)
        .Select(SliceColumn::NAME, SliceColumn::CNAME)
        .In(SliceColumn::ID, sliceIds)
        .ExcuteQuery(trackInfo.cardId, tempSlicePOVec);
    for (const auto &item : tempSlicePOVec) {
        CompeteSliceDomain cachelice;
        cachelice.id = item.id;
        cachelice.timestamp = item.timestamp;
        cachelice.endTime = item.endTime;
        cachelice.name = item.name;
        cachelice.cname = item.cname;
        competeSliceVec.emplace_back(cachelice);
    }
}
}
