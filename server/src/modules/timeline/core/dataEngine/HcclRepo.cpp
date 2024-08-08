// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#include "pch.h"
#include "TableDefs.h"
#include "TrackInfoManager.h"
#include "DataBaseManager.h"
#include "HcclRepo.h"
namespace Dic::Module::Timeline {
void HcclRepo::QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery, std::vector<SliceDomain> &sliceVec)
{
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(sliceQuery.trackId, trackInfo);
    if (!isSuccess) {
        ServerLog::Warn("hccl query all slice track info is not exist, track is: ", sliceQuery.trackId);
        return;
    }
    std::string sql;
    std::string tid;
    const std::string suffix = "group";
    auto database = DataBaseManager::Instance().GetTraceDatabase(sliceQuery.rankId);
    if (database == nullptr) {
        ServerLog::Warn("hccl open database is failed, rank is: ", sliceQuery.rankId);
        return;
    }
    if (StringUtil::EndWith(trackInfo.threadId, suffix)) {
        sql = "with tmp as (select info.opId from " + TABLE_TASK + " main join " + TABLE_COMMUNICATION_TASK_INFO +
            " info on info.globalTaskId = main.globalTaskId where main.deviceId = ?), "
            " sub as (select startNs,endNs, groupName as tid,ROWID as id from " +
            TABLE_COMMUNICATION_OP +
            " where opId in (select opId from tmp group by opId)) "
            "select id, startNs,endNs from sub where tid = ? order by startNs,id;";
        tid = trackInfo.threadId.substr(0, trackInfo.threadId.size() - suffix.size());
    } else {
        sql = "select startNs, endNs, info.planeId as tid,main.ROWID as id from " +
            TABLE_TASK + " main join " + TABLE_COMMUNICATION_TASK_INFO +
            " info on info.globalTaskId = main.globalTaskId where main.deviceId = ? and tid = ? order by startNs,id;";
        tid = trackInfo.threadId;
    }
    auto stmt = database->CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Warn("hccl query all slice get stmt failed, rank is: ", sliceQuery.rankId);
        return;
    }
    auto resultSet = stmt->ExecuteQuery(trackInfo.rankId, tid);
    if (resultSet == nullptr) {
        ServerLog::Warn("Failed to execute query hccl query all slice");
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
void HcclRepo::QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::vector<uint64_t> &sliceIds) {}
uint64_t HcclRepo::QueryPythonFunctionCountByTrackId(const SliceQuery &sliceQuery)
{
    return 0;
}
void HcclRepo::QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
    std::vector<CompeteSliceDomain> &sliceVec)
{}
void HcclRepo::QueryFlowPointByTimeRange(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) {}
void HcclRepo::QueryFlowPointByFlowId(const FlowQuery &flowQuery, std::vector<FlowPoint> &flowPointVec) {}
void HcclRepo::QueryAllThreadInfo(const ThreadQuery &flowQuery,
    std::unordered_map<uint64_t, std::pair<std::string, std::string>> &threadInfo)
{}

void HcclRepo::QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
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
    auto database = DataBaseManager::Instance().GetTraceDatabase(sliceQuery.rankId);
    if (database == nullptr) {
        ServerLog::Warn("hccl open database is failed, rank is: ", sliceQuery.rankId);
        return;
    }
    const std::string nameKey = database->GetDbPath();
    std::string sql;
    const std::string suffix = "group";
    if (StringUtil::EndWith(trackInfo.threadId, suffix)) {
        sql = "select ROWID as id, startNs,endNs,opName as name from " + TABLE_COMMUNICATION_OP +
            " where 1 = 1 and id in (";
    } else {
        sql = "select startNs, endNs, info.taskType as name,"
            " main.ROWID as id from " +
            TABLE_TASK + " main join " + TABLE_COMMUNICATION_TASK_INFO +
            " info on info.globalTaskId = main.globalTaskId where 1 = 1 and id in (";
    }
    std::string sliceidvecStr = StringUtil::join(sliceIds, ", ");
    sql += sliceidvecStr + ");";
    auto stmt = database->CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Warn("Failed to parpare hccl query slice by ids, rank is: ", sliceQuery.rankId);
        return;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Warn("Failed to execute query hccl query slice by ids");
        return;
    }
    while (resultSet->Next()) {
        CompeteSliceDomain competeSliceDomain;
        competeSliceDomain.id = resultSet->GetUint64("id");
        competeSliceDomain.timestamp = resultSet->GetUint64("startNs");
        competeSliceDomain.endTime = resultSet->GetUint64("endNs");
        competeSliceDomain.name = FullDb::DbTraceDataBase::GetStringCacheValue(nameKey, resultSet->GetString("name"));
        competeSliceVec.emplace_back(competeSliceDomain);
    }
}
}
