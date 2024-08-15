// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#include "pch.h"
#include "TableDefs.h"
#include "TrackInfoManager.h"
#include "DataBaseManager.h"
#include "CannApiRepo.h"
namespace Dic::Module::Timeline {
void CannApiRepo::QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery, std::vector<SliceDomain> &sliceVec)
{
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(sliceQuery.trackId, trackInfo);
    if (!isSuccess) {
        return;
    }
    std::vector<CannApiPO> cannApipoVec;
    cannApiTable->Select(CannApiColumn::ID, CannApiColumn::TIMESTAMP, CannApiColumn::ENDTIME)
        .Eq(CannApiColumn::TYPE, trackInfo.threadId)
        .Eq(CannApiColumn::GLOBAL_TID, trackInfo.processId)
        .ExcuteQuery(trackInfo.cardId, cannApipoVec);
    for (const auto &item : cannApipoVec) {
        SliceDomain sliceDomain;
        sliceDomain.id = item.id;
        sliceDomain.timestamp = item.timestamp;
        sliceDomain.endTime = item.endTime;
        sliceVec.emplace_back(sliceDomain);
    }
}
void CannApiRepo::QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::vector<uint64_t> &sliceIds) {}
uint64_t CannApiRepo::QueryPythonFunctionCountByTrackId(const SliceQuery &sliceQuery)
{
    return 0;
}
void CannApiRepo::QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
    std::vector<CompeteSliceDomain> &sliceVec)
{}
void CannApiRepo::QueryFlowPointByTimeRange(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) {}
void CannApiRepo::QueryFlowPointByFlowId(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) {}
void CannApiRepo::QueryAllThreadInfo(const ThreadQuery &flowQuery,
    std::unordered_map<uint64_t, std::pair<std::string, std::string>> &threadInfo)
{}

void CannApiRepo::QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
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
    const std::string nameKey = cannApiTable->GetDbPath(trackInfo.cardId);
    std::vector<CannApiPO> cannApipoVec;
    cannApiTable->Select(CannApiColumn::ID, CannApiColumn::TIMESTAMP)
        .Select(CannApiColumn::ENDTIME, CannApiColumn::NAME)
        .In(CannApiColumn::ID, sliceIds)
        .ExcuteQuery(trackInfo.cardId, cannApipoVec);
    for (const auto &item : cannApipoVec) {
        CompeteSliceDomain competeSliceDomain;
        competeSliceDomain.id = item.id;
        competeSliceDomain.timestamp = item.timestamp;
        competeSliceDomain.endTime = item.endTime;
        competeSliceDomain.name = FullDb::DbTraceDataBase::GetStringCacheValue(nameKey, std::to_string(item.name));
        competeSliceVec.emplace_back(competeSliceDomain);
    }
}

void CannApiRepo::SetCannApiTable(std::unique_ptr<CannApiTable> cannApiTablePtr)
{
    if (cannApiTablePtr != nullptr) {
        cannApiTable = std::move(cannApiTablePtr);
    }
}
}
