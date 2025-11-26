// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#include "pch.h"
#include "TableDefs.h"
#include "TrackInfoManager.h"
#include "DataBaseManager.h"
#include "OverlapAnsRepo.h"
namespace Dic::Module::Timeline {
using namespace Dic::Server;
void OverlapAnsRepo::QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
    std::vector<SliceDomain> &sliceVec)
{
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(sliceQuery.trackId, trackInfo, sliceQuery.rankId);
    if (!isSuccess) {
        ServerLog::Warn("overlap query all slice track info is not exist, track is: ", sliceQuery.trackId);
        return;
    }
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(sliceQuery.rankId);
    if (database == nullptr) {
        ServerLog::Warn("overlap open database is failed");
        return;
    }
    std::string sql = "SELECT ROWID as id, startNs, endNs from " + TABLE_OVERLAP_ANALYSIS +
        " where deviceId = ? and type = ? order by startNs , id";
    auto stmt = database->CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Warn("Failed to parpare overlap query all slice");
        return;
    }
    stmt->BindParams(trackInfo.deviceId, trackInfo.threadId);
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Warn("Failed to execute query overlap query all slice");
        return;
    }
    while (resultSet->Next()) {
        SliceDomain sliceDomain;
        sliceDomain.id = resultSet->GetUint64("id");
        sliceDomain.timestamp = resultSet->GetUint64("startNs");
        sliceDomain.endTime = resultSet->GetUint64("endNs");
        sliceVec.emplace_back(sliceDomain);
    }
}

void OverlapAnsRepo::QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
    std::vector<CompeteSliceDomain> &competeSliceVec)
{
    if (std::empty(sliceIds)) {
        return;
    }
    std::string sql = "select 'OVERLAP_ANALYSIS'||type as name, ROWID as id, startNs,"
        " endNs from " +
        TABLE_OVERLAP_ANALYSIS + " where 1 = 1 and id in (";
    std::string sliceidvecStr = StringUtil::join(sliceIds, ", ");
    sql += sliceidvecStr + ");";
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(sliceQuery.rankId);
    if (database == nullptr) {
        ServerLog::Warn("overlap open database is failed");
        return;
    }
    const std::string nameKey = database->GetDbPath();
    auto stmt = database->CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Warn("Failed to parpare overlap query slice by ids");
        return;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Warn("Failed to execute query overlap query slice by ids");
        return;
    }
    while (resultSet->Next()) {
        CompeteSliceDomain competeSlice;
        competeSlice.id = resultSet->GetUint64("id");
        competeSlice.timestamp = resultSet->GetUint64("startNs");
        competeSlice.endTime = resultSet->GetUint64("endNs");
        competeSlice.name = FullDb::DbTraceDataBase::GetStringCacheValue(nameKey, resultSet->GetString("name"));
        competeSliceVec.emplace_back(competeSlice);
    }
}

bool OverlapAnsRepo::QuerySliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain)
{
    const uint64_t sliceId = NumberUtil::TryParseUnsignedLongLong(sliceQuery.sliceId);
    std::vector<CompeteSliceDomain> sliceVec;
    QueryCompeteSliceByIds(sliceQuery, { sliceId }, sliceVec);
    if (std::empty(sliceVec)) {
        ServerLog::Warn("Failed to query overlap slice detail by id. id is: %", sliceQuery.sliceId);
        return false;
    }
    competeSliceDomain = std::move(sliceVec[0]);
    return true;
}

int OverlapAnsRepo::GetTypeByName(const std::string &name)
{
    for (size_t index = 0; index < OVERLAP_TYPES.size(); index++) {
        if (name == OVERLAP_TYPES[index]) {
            return static_cast<int>(index);
        }
    }
    return -1;
}
}
