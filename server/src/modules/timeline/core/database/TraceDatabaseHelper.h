/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACEDATABASEHELPER_H
#define PROFILER_SERVER_TRACEDATABASEHELPER_H

#include "TableDefs.h"
#include "SqlitePreparedStatement.h"
#include "VirtualTraceDatabase.h"
#include "NumberUtil.h"
#include "FullDbEnumUtil.h"
#include "CommonDefs.h"
#include "DbSqlDefs.h"
#include "JsonUtil.h"
#include "ServerLog.h"
#include "NpuInfoRepo.h"
#include "DataBaseManager.h"

namespace Dic::Module::Timeline {
using namespace Protocol;
const std::string LANE_FP_BP = "FP/BP";
const std::string LANE_P2P_OP = "P2P Op";
const std::string MARKER_FP = "FP";
const std::string MARKER_BP = "BP";
const std::string MARKER_SEND = "SEND";
const std::string MARKER_RECV = "RECV";
const std::string MARKER_BATCH_SEND_RECV = "BATCH_SEND_RECV";

const std::string QUERY_FWDBWD_FLOW_DATA_TEXT_SQL =
    "SELECT s.timestamp - ? as sStart, s.end_time - ? as sEnd, f.timestamp - ? as fStart, f.end_time - ? as fEnd "
    "From ( "
    "    SELECT f.flow_id as flow_id, f.timestamp as timestamp, s.end_time as end_time "
    "    FROM " + FLOW_TABLE + "  f JOIN " + SLICE_TABLE + " s ON f.track_id = s.track_id AND f.timestamp = s.timestamp"
    "    WHERE f.cat = 'fwdbwd' and type = 's' "
    ") s "
    "JOIN ("
    "    SELECT f.flow_id as flow_id, f.timestamp as timestamp, s.end_time as end_time "
    "    FROM " + FLOW_TABLE + "  f JOIN " + SLICE_TABLE + " s ON f.track_id = s.track_id AND f.timestamp = s.timestamp"
    "    WHERE f.cat = 'fwdbwd' and type = 'f' "
    ") f ON s.flow_id = f.flow_id "
    "WHERE s.timestamp >= ? AND s.end_time < ? ORDER by s.timestamp ";

const std::string QUERY_FWDBWD_FLOW_DATA_DB_SQL =
    "with flow_table as ( "
    "    SELECT ids.id as flowId, ids.connectionId as connectionId "
    "    FROM " + TABLE_CONNECTION_CATS + " cats JOIN " + TABLE_CONNECTION_IDS + " ids "
    "    ON cat = 'fwdbwd' and cats.connectionId = ids.connectionId "
    "), "
    "api_table as ( "
    "    SELECT startNs, endNs, connectionId FROM " + TABLE_API +
    "    WHERE type in (SELECT id FROM " + TABLE_ENUM_API_TYPE + " WHERE name = 'op')"
    "), "
    "data as ( "
    "    SELECT startNs, endNs, flow.connectionId FROM api_table api join flow_table flow "
    "    ON api.connectionId = flow.flowId ORDER BY flow.connectionId, startNs ASC "
    ") "
    "SELECT s.startNs as sStart, s.endNs as sENd, f.startNs as fStart, f.endNs as fEnd "
    "FROM data s JOIN data f ON s.connectionId = f.connectionId AND s.startNs < f.startNs "
    "WHERE s.startNs >= ? AND s.startNs < ? ORDER by s.startNs ASC";

// 目前根据通信算子名进行过滤，此种方式不够准确，待后续进一步优化为锁定通信域后便锁定p2p通信算子
const std::string QUERY_P2P_COMMUNICATION_OP_TEXT_SQL =
    "SELECT t.pid as pid, t.tid as tid, s.timestamp - ? as startTime, s.duration as duration, s.name as name "
    "FROM " + SLICE_TABLE + " s JOIN " + THREAD_TABLE + " t ON s.track_id = t.track_id WHERE s.track_id in ( "
    "    SELECT t.track_id FROM " + THREAD_TABLE + " t JOIN " + PROCESS_TABLE + " p ON t.pid = p.pid "
    "    WHERE p.process_name = 'HCCL' and t.thread_name like 'Group%' "
    ") AND ( "
    "LOWER(s.name) like 'hcom_send%' or LOWER(s.name) like 'hcom_receive%' or LOWER(s.name) like 'hcom_batchsendrecv%' "
    ") AND s.timestamp >= ? AND s.end_time <= ? ORDER BY s.timestamp ASC";
const std::string QUERY_P2P_COMMUNICATION_OP_DB_SQL =
    "SELECT task.globalPid as pid, 0 as tid, op.startNs - ? as startTime, op.endNs - op.startNs as duration, "
    "str.value as name FROM " + TABLE_COMMUNICATION_OP + " op JOIN " + TABLE_STRING_IDS + " str ON op.opName = str.id "
    "JOIN " + TABLE_TASK + " task ON op.connectionId = task.connectionId AND op.startNs = task.startNs "
    "WHERE (LOWER(str.value) like 'hcom_send%' or LOWER(str.value) like 'hcom_receive%' "
    "or LOWER(str.value) like 'hcom_batchsendrecv%') AND op.startNs >= ? AND op.endNs <= ? ORDER BY op.startNs ASC";

class TraceDatabaseHelper {
public:
/* Functions for BbTraceDataBase */
static std::optional<std::string> QueryConnectionId(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                    const Protocol::UnitFlowsParams &requestParams);

static std::unique_ptr<SqliteResultSet>
QueryThreadsByPid(std::unique_ptr<SqlitePreparedStatement> &stmt, uint64_t startTime, uint64_t endTime,
                  const Dic::Protocol::Metadata &metaData, const std::string &rankId);

static std::unique_ptr<SqliteResultSet> QueryUnitCounter(std::unique_ptr<SqlitePreparedStatement> &stmt,
      const Protocol::UnitCounterParams &requestParams, uint64_t minTimestamp, const std::string& rankId);

static std::unique_ptr<SqliteResultSet> QuerySystemViewData(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                            const Protocol::SystemViewParams &requestParams,
                                                            const std::string& rankId);

static std::unique_ptr<SqliteResultSet> QueryThreadTracesSummary(std::unique_ptr<SqlitePreparedStatement> &stmt,
        const Protocol::UnitThreadTracesSummaryParams &requestParams, const std::string& rankId, uint64_t minTimestamp);
static std::vector<uint64_t> GetDeviceIdList(const std::string &fileId);
static bool IsDeviceIdUnique(const std::string &fileId);
static void CalculateSelfTime(std::vector<Protocol::SimpleSlice> &rows,
    std::map<std::string, uint64_t> &selfTimeKeyValue, uint64_t startTime, uint64_t endTime);

static void ReduceThread(const std::vector<Protocol::SimpleSlice> &rows,
const std::map<std::string, uint64_t> &selfTimeKeyValue, Protocol::UnitThreadsBody &responseBody);

static void ReduceThread(const std::vector<CompeteSliceDomain> &rows,
    const std::map<std::string, uint64_t> &selfTimeKeyValue, Protocol::UnitThreadsBody &responseBody);
static void SetNpuInfoRepo(std::unique_ptr<NpuInfoRepo> npuInfoRepoPtr);

static inline std::vector<Protocol::SimpleSlice> ThreadsInfoFilter(
        const std::vector<Protocol::SimpleSlice> &simpleSliceVec, uint64_t startTime, uint64_t endTime)
{
    std::vector<Protocol::SimpleSlice> nRows;
    for (auto &row : simpleSliceVec) {
        if (row.timestamp <= endTime && row.endTime >= startTime) {
            nRows.emplace_back(row);
        }
    }
    return nRows;
}

template <typename... Args>
static inline std::unique_ptr<SqliteResultSet> Execute(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                       Args&&... args)
{
    stmt->BindParams(std::forward<Args>(args)...);
    auto result = stmt->ExecuteQuery();
    if (result == nullptr) {
        throw DatabaseException("Failed to ExecuteQuery.");
    }
    return result;
};

template <typename... Args>
static inline std::unique_ptr<SqliteResultSet> ExecuteQuery(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                            const std::string &sql, Args&&... args)
{
    Prepare(stmt, sql);
    return Execute(stmt, std::forward<Args>(args)...);
};

static inline std::unique_ptr<SqlitePreparedStatement>& Prepare(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                               const std::string &sql)
{
    if (stmt == nullptr) {
        throw DatabaseException("Failed to prepare sql.");
    }
    if (!stmt->Prepare(sql)) {
        throw DatabaseException("Failed to prepare sql.");
    }
    stmt->Reset();
    return stmt;
};

static inline PROCESS_TYPE GetProcessType(const std::string &metaType)
{
    auto processType = STR_TO_ENUM<PROCESS_TYPE>(metaType);
    if (!processType.has_value()) {
        return static_cast<PROCESS_TYPE>(atoi(metaType.c_str()));
    }
    return processType.value();
}
static std::unique_ptr<SqliteResultSet> QueryThreadSameOperatorsDetails(std::unique_ptr<SqlitePreparedStatement> &stmt,
     const Protocol::UnitThreadsOperatorsParams &requestParams, const std::string& rankId,
     uint64_t minTimestamp, const std::string& orderBy);
static bool QueryEventsViewData4Db(std::unique_ptr <SqlitePreparedStatement> &stmt,
    const Protocol::EventsViewParams &params, Protocol::EventsViewBody &body, uint64_t minTimestamp,
    const std::string& rankId);
/* Functions for JsonTraceDataBase */
static bool QueryEventsViewData4Text(std::unique_ptr <SqlitePreparedStatement> &stmt,
    const Protocol::EventsViewParams &params, Protocol::EventsViewBody &body, uint64_t minTimestamp);
static void QueryAllSliceInRangeByTrackIdHelper(std::unique_ptr<SqliteResultSet> &resultSet,
    uint64_t unitTime, uint64_t minTimestamp, Protocol::UnitThreadTracesSummaryBody &responseBody);
static void SetKernelDetailHelpler(std::unique_ptr<SqliteResultSet> resultSet, uint64_t minTimestamp,
                            Protocol::KernelDetailsBody &responseBody);
static void FilterTopLevelApi(std::vector<Protocol::FlowLocation> &originData, const std::set<std::string> &pattern,
    std::vector<Protocol::FlowLocation> &filterData, std::vector<uint32_t> &indexes);
// 内部接口不对外，调用处保证stmt不为空
static bool ExecuteQueryFwdBwdDataByFlow(std::unique_ptr<SqlitePreparedStatement> stmt,
    const std::string &rankId, uint64_t offset, const Protocol::ExtremumTimestamp &range,
    std::vector<Protocol::ThreadTraces> &fwdBwdData);
static bool ExecuteQueryP2POpData(std::unique_ptr<SqlitePreparedStatement> stmt, const std::string &rankId,
    uint64_t offset, const ExtremumTimestamp &range, std::vector<Protocol::ThreadTraces> &p2pOpData);
static inline bool IsValidHCCLGroupNameValue(const std::string &groupNameValue)
{
    const std::string regexStr = "[^0-9]";
    const std::regex pattern(regexStr);
    return std::regex_search(groupNameValue, pattern);
}
private:
/* Functions for BbTraceDataBase */
    static inline std::unique_ptr<NpuInfoRepo> npuInfoRepo = std::make_unique<NpuInfoRepo>();
    static inline void DealLastData(std::vector<Protocol::SimpleSlice> &rows,
                                    std::map<std::string, uint64_t> &selfTimeKeyValue,
                                    uint64_t startTime, uint64_t endTime, uint64_t index)
    {
        while (++index < rows.size()) {
            if (rows.at(index).timestamp <= endTime && rows.at(index).endTime >= startTime) {
                AddData(selfTimeKeyValue, rows.at(index).name, rows.at(index).duration);
            }
        }
    }

    static inline void AddData(std::map<std::string, uint64_t> &selfTimeKeyValue, const std::string &name,
                               uint64_t tmpSelfTime)
    {
        if (selfTimeKeyValue.find(name) != selfTimeKeyValue.end()) {
            selfTimeKeyValue.at(name) = selfTimeKeyValue.at(name) + tmpSelfTime;
        } else {
            selfTimeKeyValue.emplace(name, tmpSelfTime);
        }
    }

    static std::string GetOrderByCondition(const EventsViewParams &params);
    static inline void CalculateFwdBwdDataByFlow(const FlowStartAndEndTime &start, const FlowStartAndEndTime &end,
        uint8_t index, const std::string &rankId, std::vector<Protocol::ThreadTraces> &fwdBwdData);
    static std::string GetSystemViewSqlByLayer(const std::string &layer, const std::string &rankId);
};
};

#endif // PROFILER_SERVER_TRACEDATABASEHELPER_H