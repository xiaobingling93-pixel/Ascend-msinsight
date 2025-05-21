// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#include "pch.h"
#include "TableDefs.h"
#include "TrackInfoManager.h"
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

bool CannApiRepo::QuerySliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain)
{
    std::vector<CannApiPO> cannApipoVec;
    cannApiTable->Select(CannApiColumn::ID, CannApiColumn::TIMESTAMP)
        .Select(CannApiColumn::ENDTIME, CannApiColumn::NAME)
        .Select(CannApiColumn::GLOBAL_TID, CannApiColumn::TYPE)
        .Eq(CannApiColumn::ID, sliceQuery.sliceId)
        .ExcuteQuery(sliceQuery.rankId, cannApipoVec);
    if (std::empty(cannApipoVec)) {
        ServerLog::Warn("Failed to query CANN slice detail by id. id is: %", sliceQuery.sliceId);
        return false;
    }
    CannApiPO target = cannApipoVec[0];
    competeSliceDomain.id = target.id;
    competeSliceDomain.timestamp = target.timestamp;
    competeSliceDomain.endTime = target.endTime;
    std::unordered_map<uint64_t, std::string> strMap = stringIdsTable->QueryStrMap({ target.name }, sliceQuery.rankId);
    competeSliceDomain.name = strMap[target.name];
    std::unordered_map<uint64_t, std::string> levelMap = apiTypeTable->QueryStrMap({ target.type }, sliceQuery.rankId);
    std::string level = levelMap[target.type];
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    JsonUtil::AddConstMember(json, CannApiColumn::GLOBAL_TID, std::to_string(target.globalTid), allocator);
    JsonUtil::AddConstMember(json, CannApiColumn::TYPE, level, allocator);
    JsonUtil::AddConstMember(json, CannApiColumn::NAME, competeSliceDomain.name, allocator);
    JsonUtil::AddConstMember(json, CannApiColumn::ID, std::to_string(target.id), allocator);
    competeSliceDomain.args = JsonUtil::JsonDump(json);
    return true;
}

bool CannApiRepo::QuerySliceDetailInfoByNameList(const SliceQueryByNameList &params,
                                                 std::vector<CompeteSliceDomain> &res)
{
    // 根据名字查询stringId的内容
    std::unordered_map<uint64_t, std::string> strMap =
            stringIdsTable->QueryStrMapByValues(params.nameList, params.rankId);
    if (strMap.empty()) {
        return false;
    }
    std::vector<uint64_t> stringIds;
    std::transform(strMap.begin(), strMap.end(), std::back_inserter(stringIds),
        [](const std::pair<uint64_t, std::string>& pair) { return pair.first; });
    // 根据stringIds查询算子
    std::vector<CannApiPO> cannApipoVec;
    cannApiTable->Select(CannApiColumn::NAME, CannApiColumn::TIMESTAMP, CannApiColumn::ENDTIME)
            .In(CannApiColumn::NAME, stringIds)
            .OrderBy(CannApiColumn::TIMESTAMP, TableOrder::ASC)
            .ExcuteQuery(params.rankId, cannApipoVec);
    for (const auto &item: cannApipoVec) {
        CompeteSliceDomain domain;
        domain.name = strMap[item.name];
        domain.timestamp = item.timestamp;
        domain.endTime = item.endTime;
        res.push_back(domain);
    }
    return true;
}
}
