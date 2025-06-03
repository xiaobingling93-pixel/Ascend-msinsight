// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#include "pch.h"
#include "TableDefs.h"
#include "TrackInfoManager.h"
#include "DataBaseManager.h"
#include "PythonGcRepo.h"
namespace Dic::Module::Timeline {
void PythonGcRepo::QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
                                                        std::vector<SliceDomain> &sliceVec)
{
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(sliceQuery.trackId, trackInfo);
    if (!isSuccess) {
        ServerLog::Warn("gcRecord query all slice track info is not exist, track is: ", sliceQuery.trackId);
        return;
    }
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(sliceQuery.rankId);
    if (database == nullptr) {
        ServerLog::Error("gcRecord open database is failed");
        return;
    }
    std::string sql =
        "SELECT ROWID as id, startNs, endNs from " + GC_RECORD + " where globalTid = ? order by startNs , id";
    auto stmt = database->CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to parpare gcRecord query all slice");
        return;
    }
    stmt->BindParams(trackInfo.processId);
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to execute query gcRecord query all slice");
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

void PythonGcRepo::QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::vector<uint64_t> &sliceIds) {}
uint64_t PythonGcRepo::QueryPythonFunctionCountByTrackId(const SliceQuery &sliceQuery)
{
    return 0;
}
void PythonGcRepo::QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
    std::vector<CompeteSliceDomain> &sliceVec)
{}

void PythonGcRepo::QueryAllThreadInfo(const ThreadQuery &flowQuery,
    std::unordered_map<uint64_t, std::pair<std::string, std::string>> &threadInfo)
{}

void PythonGcRepo::QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
    std::vector<CompeteSliceDomain> &competeSliceVec)
{
    if (std::empty(sliceIds)) {
        return;
    }
    std::string sql = "select 'Python GC' as name, gc.ROWID as id, startNs, endNs "
        " from " +
        GC_RECORD +
        "  gc  "
        "    where 1 = 1 and id in (";
    std::string sliceidvecStr = StringUtil::join(sliceIds, ", ");
    sql += sliceidvecStr + ");";
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(sliceQuery.rankId);
    if (database == nullptr) {
        ServerLog::Error("gcRecord open database is failed");
        return;
    }
    const std::string nameKey = database->GetDbPath();
    auto stmt = database->CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to parpare gcRecord query slice by ids");
        return;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("gcRecord query slice by ids is failed! ");
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

bool PythonGcRepo::QuerySliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain)
{
    std::vector<PythonGCPO> pythonGCPOs;
    table->Select(PythonGCColumn::ID, PythonGCColumn::TIMESTAMP)
            .Select(PythonGCColumn::ENDTIME)
            .ExcuteQuery(sliceQuery.rankId, pythonGCPOs);
    if (std::empty(pythonGCPOs)) {
        ServerLog::Warn("Failed to query pythonGC slice detail by id. id is: %", sliceQuery.sliceId);
        return false;
    }
    competeSliceDomain.id = pythonGCPOs[0].id;
    competeSliceDomain.timestamp = pythonGCPOs[0].timestamp;
    competeSliceDomain.endTime = pythonGCPOs[0].endTime;
    competeSliceDomain.name = "PythonGC";
    return true;
}
}