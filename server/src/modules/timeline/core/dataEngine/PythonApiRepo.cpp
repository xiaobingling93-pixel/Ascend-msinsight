// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#include "pch.h"
#include "TableDefs.h"
#include "TrackInfoManager.h"
#include "PythonApiRepo.h"
namespace Dic::Module::Timeline {
void PythonApiRepo::QuerySimpleSliceWithOutNameByTrackId(const SliceQuery &sliceQuery,
    std::vector<SliceDomain> &sliceVec)
{
    TrackInfo trackInfo;
    auto &instance = TrackInfoManager::Instance();
    const bool isSuccess = instance.GetTrackInfo(sliceQuery.trackId, trackInfo);
    if (!isSuccess) {
        ServerLog::Warn("python api query all slice track info is not exist, track is: ", sliceQuery.trackId);
        return;
    }
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(sliceQuery.rankId);
    if (database == nullptr) {
        ServerLog::Warn("python api open database is failed");
        return;
    }
    std::string sql =
        "SELECT ROWID as id, startNs, endNs from " + TABLE_API + " where globalTid = ? order by startNs , id";
    auto stmt = database->CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Warn("Failed to parpare python api query all slice");
        return;
    }
    stmt->BindParams(trackInfo.processId);
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Warn("Failed to execute query python api query all slice");
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
void PythonApiRepo::QuerySliceIdsByCat(const SliceQuery &sliceQuery, std::vector<uint64_t> &sliceIds)
{
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(sliceQuery.trackId, trackInfo);
    if (!isSuccess) {
        ServerLog::Warn("python api query slice by cat track info is not exist, track is: ", sliceQuery.trackId);
        return;
    }
    std::string sql = "SELECT ROWID as id from " + TABLE_API + " where globalTid = ? and type = 50003";
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(sliceQuery.rankId);
    if (database == nullptr) {
        ServerLog::Warn("python api open database is failed");
        return;
    }
    auto stmt = database->CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Warn("Failed to parpare python api query slice by cat");
        return;
    }
    stmt->BindParams(trackInfo.processId);
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Warn("Failed to execute query python api query slice by cat");
        return;
    }
    while (resultSet->Next()) {
        sliceIds.emplace_back(resultSet->GetUint64("id"));
    }
}
uint64_t PythonApiRepo::QueryPythonFunctionCountByTrackId(const SliceQuery &sliceQuery)
{
    TrackInfo trackInfo;
    const bool isSuccess = TrackInfoManager::Instance().GetTrackInfo(sliceQuery.trackId, trackInfo);
    uint64_t count = 0;
    if (!isSuccess) {
        ServerLog::Warn("python api query python function slice track info is not exist, track is: ",
            sliceQuery.trackId);
        return count;
    }
    std::string sql = "SELECT count(*) as count from " + TABLE_API + " where globalTid = ? and type = 50003";
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(sliceQuery.rankId);
    if (database == nullptr) {
        ServerLog::Warn("python api open database is failed");
        return 0;
    }
    auto stmt = database->CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Warn("Failed to parpare python api query python function");
        return 0;
    }
    stmt->BindParams(trackInfo.processId);
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Warn("Failed to execute query python api query python function");
        return 0;
    }
    while (resultSet->Next()) {
        count = resultSet->GetUint64("count");
    }
    return count;
}
void PythonApiRepo::QueryCompeteSliceVecByTimeRangeAndTrackId(const SliceQuery &sliceQuery,
    std::vector<CompeteSliceDomain> &sliceVec)
{}

void PythonApiRepo::QueryAllThreadInfo(const ThreadQuery &flowQuery,
    std::unordered_map<uint64_t, std::pair<std::string, std::string>> &threadInfo)
{}

void PythonApiRepo::QueryCompeteSliceByIds(const SliceQuery &sliceQuery, const std::vector<uint64_t> &sliceIds,
    std::vector<CompeteSliceDomain> &competeSliceVec)
{
    if (std::empty(sliceIds)) {
        return;
    }
    std::string sql = "select name, ROWID as id, startNs, endNs "
        " from " +
        TABLE_API + " where 1 = 1 and id in (";
    std::string sliceidvecStr = StringUtil::join(sliceIds, ", ");
    sql += sliceidvecStr + ");";
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(sliceQuery.rankId);
    if (database == nullptr) {
        ServerLog::Warn("python api open database is failed");
        return;
    }
    const std::string nameKey = database->GetDbPath();
    auto stmt = database->CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Warn("Failed to parpare python api query slice by ids");
        return;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Warn("Failed to execute query python api query slice by ids");
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

bool PythonApiRepo::QuerySliceDetailInfo(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain)
{
    std::vector<PytorchApiPO> apiPOs;
    pytorchApiTable->Select(PytorchApiColumn::ID, PytorchApiColumn::TIMESTAMP)
        .Select(PytorchApiColumn::ENDTIME, PytorchApiColumn::NAME)
        .Select(PytorchApiColumn::SEQUENCE_NUMBER, PytorchApiColumn::FWD_THREAD_ID)
        .Select(PytorchApiColumn::INPUT_DTYPES, PytorchApiColumn::INPUT_SHAPES)
        .Select(PytorchApiColumn::CALL_CHAIN_ID, PytorchApiColumn::CONNECTIONID)
        .Eq(PytorchApiColumn::ID, sliceQuery.sliceId)
        .ExcuteQuery(sliceQuery.rankId, apiPOs);
    if (std::empty(apiPOs)) {
        ServerLog::Warn("Failed to query pytorch api slice detail by id. id is: %", sliceQuery.sliceId);
        return false;
    }
    const PytorchApiPO target = apiPOs[0];
    competeSliceDomain.id = target.id;
    competeSliceDomain.timestamp = target.timestamp;
    competeSliceDomain.endTime = target.endTime;
    QuerySliceArgs(sliceQuery, competeSliceDomain, target);
    return true;
}

void PythonApiRepo::QuerySliceArgs(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain,
    const PytorchApiPO &target)
{
    std::vector<uint64_t> strIds;
    strIds.emplace_back(target.name);
    if (!std::empty(target.inputShapes)) {
        strIds.emplace_back(NumberUtil::TryParseInt(target.inputShapes));
    }
    if (!std::empty(target.inputDtypes)) {
        strIds.emplace_back(NumberUtil::TryParseInt(target.inputDtypes));
    }
    std::unordered_map<uint64_t, std::string> strMap = stringIdsTable->QueryStrMap(strIds, sliceQuery.rankId);
    const std::string correlationId = std::to_string(target.connectionId);
    const std::string inputShapeName = strMap[NumberUtil::TryParseInt(target.inputShapes)];
    const std::string inputTypeName = strMap[NumberUtil::TryParseInt(target.inputDtypes)];
    competeSliceDomain.name = strMap[target.name];
    std::string callStack = QuerySliceCallStack(sliceQuery, target);
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    if (!std::empty(target.sequenceNumber)) {
        JsonUtil::AddConstMember(json, PytorchApiColumn::SEQUENCE_NUMBER, target.sequenceNumber, allocator);
    }
    if (!std::empty(target.fwdThreadId)) {
        JsonUtil::AddConstMember(json, PytorchApiColumn::FWD_THREAD_ID, target.fwdThreadId, allocator);
    }
    if (!std::empty(correlationId)) {
        JsonUtil::AddConstMember(json, PytorchApiColumn::CONNECTIONID, correlationId, allocator);
    }
    if (!std::empty(inputShapeName)) {
        JsonUtil::AddConstMember(json, PytorchApiColumn::INPUT_SHAPES, inputShapeName, allocator);
    }
    if (!std::empty(inputTypeName)) {
        JsonUtil::AddConstMember(json, PytorchApiColumn::INPUT_DTYPES, inputTypeName, allocator);
    }
    if (!std::empty(callStack)) {
        JsonUtil::AddConstMember(json, "Call stack", callStack, allocator);
    }
    competeSliceDomain.args = JsonUtil::JsonDump(json);
}

std::string PythonApiRepo::QuerySliceCallStack(const SliceQuery &sliceQuery, const PytorchApiPO &target)
{
    std::string callStack;
    if (!std::empty(target.callchainId)) {
        std::vector<PytorchCallchainsPO> chainPOs;
        pytorchCallchainsTable->Select(PytorchCallchainsColumn::STACK)
            .Eq(PytorchCallchainsColumn::ID, target.callchainId)
            .OrderBy(PytorchCallchainsColumn::STACK_DEPTH, TableOrder::ASC)
            .ExcuteQuery(sliceQuery.rankId, chainPOs);
        std::vector<uint64_t> callStrIds;
        callStrIds.reserve(chainPOs.size());
        for (const auto &item : chainPOs) {
            callStrIds.emplace_back(item.stack);
        }
        std::unordered_map<uint64_t, std::string> callStrMap =
            stringIdsTable->QueryStrMap(callStrIds, sliceQuery.rankId);
        for (const auto &item : chainPOs) {
            callStack += callStrMap[item.stack];
            callStack += ";\n";
        }
    }
    return callStack;
}

bool PythonApiRepo::QuerySliceByTimepointAndName(const SliceQuery &sliceQuery, CompeteSliceDomain &competeSliceDomain)
{
    std::vector<StringIdsPO> strPOs;
    stringIdsTable->Select(StringIdsColumn::ID)
        .Eq(StringIdsColumn::VALUE, sliceQuery.name)
        .ExcuteQuery(sliceQuery.rankId, strPOs);
    if (std::empty(strPOs)) {
        ServerLog::Warn("Failed to query pytorch slice name by time point in db scene!");
        return false;
    }
    std::vector<uint64_t> strIds(strPOs.size());
    std::transform(strPOs.begin(), strPOs.end(), strIds.begin(),
                   [](const StringIdsPO &item) { return item.id; });
    std::vector<PytorchApiPO> apiPOs;
    pytorchApiTable->Select(PytorchApiColumn::ID, PytorchApiColumn::TIMESTAMP)
        .Select(PytorchApiColumn::ENDTIME, PytorchApiColumn::GLOBAL_TID)
        .LessEq(PytorchApiColumn::TIMESTAMP, sliceQuery.timePoint)
        .GreaterEq(PytorchApiColumn::ENDTIME, sliceQuery.timePoint)
        .In(PytorchApiColumn::NAME, strIds)
        .OrderBy(PytorchApiColumn::TIMESTAMP, Timeline::TableOrder::DESC)
        .ExcuteQuery(sliceQuery.rankId, apiPOs);
    if (std::empty(apiPOs)) {
        ServerLog::Warn("Failed to query pytorch slice by time point in db scene!");
        return false;
    }
    const PytorchApiPO target = apiPOs[0];
    competeSliceDomain.id = target.id;
    competeSliceDomain.timestamp = target.timestamp;
    competeSliceDomain.endTime = target.endTime;
    competeSliceDomain.pid = std::to_string(target.globalTid);
    competeSliceDomain.tid = pythonApiTid;
    competeSliceDomain.cardId = TrackInfoManager::Instance().GetHostCardId(sliceQuery.rankId);
    competeSliceDomain.trackId = TrackInfoManager::Instance().GetTrackId(competeSliceDomain.cardId,
        competeSliceDomain.pid, competeSliceDomain.tid);
    competeSliceDomain.duration = target.endTime - target.timestamp;
    return true;
}
}
