//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
//
#include "OSRTApiRepo.h"
#include "TrackInfoManager.h"
#include "DataBaseManager.h"
namespace Dic::Module::Timeline {
using namespace Dic::Server;
void OSRTApiRepo::QuerySimpleSliceWithOutNameByTrackId(const Dic::Module::Timeline::SliceQuery &sliceQuery,
                                                       std::vector<SliceDomain> &sliceVec)
{
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(sliceQuery.trackId, trackInfo);
    if (!isSuccess) {
        ServerLog::Error("OSRT_API query all slice track info does not exist, track is: ", sliceQuery.trackId);
        return;
    }
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(sliceQuery.rankId);
    if (!database) {
        ServerLog::Error("OSRT_API open database failed.");
        return;
    }
    QuerySimpleSliceWithOutNameByTrackIdExecuteSQL(database, trackInfo.processId, sliceVec);
}

void OSRTApiRepo::QuerySimpleSliceWithOutNameByTrackIdExecuteSQL(std::shared_ptr<VirtualTraceDatabase> database,
    std::string &processId, std::vector<SliceDomain> &sliceVec)
{
    std::string sql = "SELECT rowid AS id, startNs, endNs FROM " + TABLE_OSRT_API + " WHERE globalTid = ?;";
    auto stmt = database->CreatPreparedStatement(sql);
    if (!stmt) {
        ServerLog::Error("Failed to prepare OSRT_API query for simple slice.");
        return;
    }
    stmt->BindParams(processId);
    auto resultSet = stmt->ExecuteQuery();
    if (!resultSet) {
        ServerLog::Error("Failed to execute OSRT_API query for simple slice.");
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

void OSRTApiRepo::QueryCompeteSliceByIds(const Dic::Module::Timeline::SliceQuery &sliceQuery,
                                         const std::vector<uint64_t> &sliceIds,
                                         std::vector<CompeteSliceDomain> &competeSliceVec)
{
    if (std::empty(sliceIds)) {
        return;
    }
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(sliceQuery.rankId);
    if (!database) {
        ServerLog::Error("OSRT_API open database failed.");
        return;
    }
    QueryCompeteSliceByIdsExecuteSQL(database, sliceIds, competeSliceVec);
}

void OSRTApiRepo::QueryCompeteSliceByIdsExecuteSQL(std::shared_ptr<VirtualTraceDatabase> database,
                                                   const std::vector<uint64_t> &sliceIds,
                                                   std::vector<CompeteSliceDomain> &competeSliceVec)
{
    std::string sliceidvecStr = StringUtil::join(sliceIds, ", ");
    std::string sql = "SELECT t2.value AS name, t1.rowid AS id, t1.startNs AS startNs, t1.endNs AS endNs FROM " +
        TABLE_OSRT_API + " AS t1 INNER JOIN " + TABLE_STRING_IDS + " AS t2 ON t1.name = t2.id "
        " WHERE t1.rowid IN (" + sliceidvecStr + ");";
    auto stmt = database->CreatPreparedStatement(sql);
    if (!stmt) {
        ServerLog::Error("Failed to prepare OSRT_API query for complete slice by ids.");
        return;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (!resultSet) {
        ServerLog::Error("Failed to execute OSRT_API query for complete slice by ids.");
        return;
    }
    while (resultSet->Next()) {
        CompeteSliceDomain competeSlice;
        competeSlice.id = resultSet->GetUint64("id");
        competeSlice.timestamp = resultSet->GetUint64("startNs");
        competeSlice.endTime = resultSet->GetUint64("endNs");
        competeSlice.name = resultSet->GetString("name");
        competeSliceVec.emplace_back(competeSlice);
    }
}

bool OSRTApiRepo::QuerySliceDetailInfo(const Dic::Module::Timeline::SliceQuery &sliceQuery,
                                       Dic::Module::Timeline::CompeteSliceDomain &competeSliceDomain)
{
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(sliceQuery.rankId);
    if (!database) {
        ServerLog::Error("OSRT_API open database failed.");
        return false;
    }
    return QuerySliceDetailInfoExecuteSQL(database, sliceQuery.sliceId, competeSliceDomain);
}

bool OSRTApiRepo::QuerySliceDetailInfoExecuteSQL(std::shared_ptr<VirtualTraceDatabase> database,
                                                 const std::string &sliceId,
                                                 Dic::Module::Timeline::CompeteSliceDomain &competeSliceDomain)
{
    std::string sql = "SELECT t2.value AS name, t1.rowid AS id, t1.startNs AS startNs, t1.endNs AS endNs FROM " +
        TABLE_OSRT_API + " AS t1 INNER JOIN " + TABLE_STRING_IDS + " AS t2 ON t1.name = t2.id "
        " WHERE t1.rowid = " + sliceId + ";";
    auto stmt = database->CreatPreparedStatement(sql);
    if (!stmt) {
        ServerLog::Error("Failed to prepare OSRT_API query for detail info.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (!resultSet) {
        ServerLog::Error("Failed to execute OSRT_API query for detail info.");
        return false;
    }
    while (resultSet->Next()) {
        competeSliceDomain.id = resultSet->GetUint64("id");
        competeSliceDomain.timestamp = resultSet->GetUint64("startNs");
        competeSliceDomain.endTime = resultSet->GetUint64("endNs");
        competeSliceDomain.name = resultSet->GetString("name");
        competeSliceDomain.args = "";
    }
    return true;
}
}