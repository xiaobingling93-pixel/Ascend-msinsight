/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_TEXTSQLCONSTANT_H
#define PROFILER_SERVER_TEXTSQLCONSTANT_H
#include <string>
#include <utility>
#include "StringUtil.h"
#include "ServerLog.h"
#include "TimelineProtocolRequest.h"
#include "TableDefs.h"
// LCOV_EXCL_BR_START
namespace Dic::Module::Timeline {
const int CACHE_SIZE = 1000;
const std::string SLICE_TABLE = "slice";
const std::string THREAD_TABLE = "thread";
const std::string PROCESS_TABLE = "process";
const std::string FLOW_TABLE = "flow";
const std::string COUNTER_TABLE = "counter";
const std::string TRACKID_TIME_INDEX = "track_id_timestamp_end_time_index";
const std::string TRACKID_CAT_INDEX = "track_id_cat_index";
const std::string FLOW_INDEX = "flow_id_time_index";
const std::string KERNEL_DETAIL = "kernel_detail";

const std::string UPDATE_PROCESS_NAME_SQL = "INSERT INTO  process  (pid, process_name) VALUES (?, ?) ON CONFLICT (pid) "
    "DO UPDATE SET process_name = excluded.process_name;";
const std::string SIMULATION_UPDATE_PROCESS_NAME_SQL =
    "INSERT INTO  process  (pid, process_name) VALUES (?, ?) ON CONFLICT (pid) "
    "DO UPDATE SET process_name = CASE WHEN process_name IS NULL OR process_name = '' THEN EXCLUDED.process_name ELSE "
    "process_name END;";
const std::string UPDATE_PROCESS_LABLE_SQL =
    "INSERT INTO process (pid, label) VALUES (?, ?) ON CONFLICT (pid) DO UPDATE SET label = excluded.label;";
const std::string UPDATE_PROCESS_SORTINDEX_SQL = "INSERT INTO process (pid, process_sort_index) VALUES (?, ?) ON "
    "CONFLICT (pid) DO UPDATE SET process_sort_index = "
    "excluded.process_sort_index;";
const std::string UPDATE_THREAD_NAME_SQL = "INSERT INTO thread (track_id, tid, pid, thread_name) VALUES (?, ?, ?, "
    "?) ON CONFLICT (track_id) DO UPDATE SET tid "
    "= excluded.tid, pid = excluded.pid, thread_name = excluded.thread_name;";
const std::string UPDATE_THREAD_SORTINDEX_SQL = "INSERT INTO thread (track_id, thread_sort_index) VALUES (?, ?) ON "
    "CONFLICT (track_id) DO UPDATE SET thread_sort_index = "
    "excluded.thread_sort_index;";
const std::string SIMULATION_UPDATE_THREAD_NAME_SQL = "INSERT INTO thread"
    " (track_id, tid, pid, thread_name, thread_sort_index) VALUES (?, ?, ?, ?, ?)"
    " ON CONFLICT (track_id) DO UPDATE SET tid = CASE WHEN tid IS NULL OR tid = '' THEN EXCLUDED.tid ELSE tid END,"
    " pid = CASE WHEN pid IS NULL OR pid = '' THEN EXCLUDED.pid ELSE pid END,"
    " thread_name = CASE WHEN thread_name IS NULL OR thread_name = '' THEN EXCLUDED.thread_name ELSE thread_name END,"
    " thread_sort_index = CASE WHEN thread_sort_index IS NULL OR thread_sort_index = 0 THEN EXCLUDED.thread_sort_index "
    "ELSE thread_sort_index END;";
const std::string CREATE_TABLE_SQL = "CREATE TABLE " + SLICE_TABLE +
    " (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp INTEGER, duration INTEGER,"
    " name TEXT, depth INTEGER, track_id INTEGER, cat TEXT, args TEXT, cname TEXT, end_time INTEGER, flag_id TEXT);" +
    "CREATE TABLE " + THREAD_TABLE + " (track_id INTEGER PRIMARY KEY, tid TEXT, pid TEXT, thread_name TEXT," +
    " thread_sort_index INTEGER);" + "CREATE TABLE " + PROCESS_TABLE +
    " (pid TEXT PRIMARY KEY, process_name TEXT, label TEXT," + " process_sort_index INTEGER);" + "CREATE TABLE " +
    FLOW_TABLE + " (id INTEGER PRIMARY KEY AUTOINCREMENT, flow_id TEXT, name TEXT, cat TEXT," +
    " track_id INTEGER, timestamp INTEGER, type TEXT);" + "CREATE TABLE " + COUNTER_TABLE +
    " (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, pid TEXT," + "timestamp INTEGER, cat TEXT, args TEXT);";

const std::string CREATE_INDEX_SQL = "CREATE INDEX " + TRACKID_TIME_INDEX + " ON " + SLICE_TABLE +
    " (timestamp, end_time, track_id);" + "CREATE INDEX " + TRACKID_CAT_INDEX + " ON " + SLICE_TABLE +
    " (track_id, cat);" + "CREATE INDEX " + FLOW_INDEX + " ON " + FLOW_TABLE + " (cat, type);";
const std::string QUERY_FLOW_BY_FLOWID_SQL = "SELECT name, cat, flow_id as flowId, timestamp, type, track_id as trackId"
    " FROM " + FLOW_TABLE + " WHERE flow_id = ?";
const std::string QUERY_ALL_THREAD_SQL = "SELECT track_id as trackId, tid, pid FROM " + THREAD_TABLE + " ;";
const std::string QUERY_SLICE_BY_ID_SQL = "SELECT track_id, flag_id FROM " + SLICE_TABLE + " WHERE id = ?";
const std::string QUERY_SLICE_BY_FLAG_ID_SQL = "SELECT id"
    " FROM " + SLICE_TABLE + " WHERE flag_id = ? AND track_id = ?;";
const std::string QUERY_UNITS_META_SQL =
    "SELECT pt.pid, pt.process_name AS processName, pt.label, pt.tid, pt.thread_name AS threadName, "
    "pt.name, pt.args, pt.track_id as trackId "
    "FROM ( "
    "    SELECT p.pid, CASE WHEN p.process_name IS NULL THEN 'Process ' || p.pid ELSE p.process_name END AS "
    "    process_name,p.label,p.process_sort_index,t.tid,t.thread_name,t.track_id,t.thread_sort_index,c.name,c.args "
    "    FROM " + PROCESS_TABLE + " p LEFT JOIN  " + THREAD_TABLE + " t ON p.pid = t.pid "
    "    LEFT JOIN ( SELECT pid, name, args FROM " + COUNTER_TABLE + " GROUP BY name, pid ) c ON c.pid = p.pid "
    ") AS pt WHERE pt.process_name IS NOT NULL "
    " ORDER BY pt.process_sort_index ASC,pt.process_name ASC,pt.pid ASC, pt.thread_sort_index ASC, pt.name ASC;";
const std::string QUERY_EXETREME_TIME_SQL = "SELECT  min(minTimestamp) AS totalMinTimestamp, max(maxTimestamp) AS "
    "totalMaxTimestamp FROM ("
    "    SELECT min(timestamp) as minTimestamp, max(timestamp) as maxTimestamp FROM " + SLICE_TABLE +
    "    UNION SELECT min(timestamp) as minTimestamp, max(timestamp) as maxTimestamp FROM " + COUNTER_TABLE +
    ")";
const std::string QUERY_UNIT_COUNTER_SQL = "SELECT timestamp - ? as startTime, args FROM " + COUNTER_TABLE + " "
    "WHERE pid = ? AND name = ? AND startTime >= ? AND startTime <= ? ORDER BY timestamp ASC";
const std::string QUERY_LAYER_DATA_SQL = "SELECT sum(case when name != 'Communication' then duration else 0 end) "
    "AS totalTime, count(distinct name) FROM slice WHERE lower(name) LIKE lower(?) and slice.track_id IN "
    "( SELECT track_id FROM process JOIN thread t ON process.pid = t.pid WHERE process_name = ? ) ";
const std::string QUERY_QUERY_TYPE_SQL =
    "SELECT DISTINCT accelerator_core FROM " + KERNEL_DETAIL + " ORDER BY accelerator_core";

const std::string QUERY_AFFINITY_API_TEXT_SQL =
    "SELECT s.track_id as track, s.id as id, s.name as name, s.timestamp - ? as startTime, "
    "s.end_time - ? as endTime, t.pid as pid, t.tid as tid "
    "FROM " + SLICE_TABLE + " s JOIN " + THREAD_TABLE + " t on s.track_id = t.track_id "
    "WHERE s.cat = 'cpu_op' AND s.name LIKE 'aten::%' OR s.name LIKE 'npu::%' "
    "ORDER BY s.track_id ASC, s.timestamp ASC";
const std::string QUERY_AFFINITY_API_DB_SQL =
    "SELECT py.ROWID as id, str.value as name, py.startNs - ? as startTime, "
    "py.endNs - ? as endTime, py.globalTid as pid, 'pytorch' as tid, py.depth as depth "
    "FROM " + TABLE_API + " py JOIN " + TABLE_STRING_IDS + " str ON py.name = str.id "
    "WHERE str.value LIKE 'aten::%' OR str.value LIKE 'npu::%' ORDER BY py.globalTid ASC, py.startNs ASC ";

    const std::string QUERY_OVERLAP_ANALYSIS_BY_TYPE_TEXT_SQL =
        "SELECT name, timestamp - ? as startNs, end_time - ? as endNs, duration FROM " + SLICE_TABLE + " "
        "WHERE track_id in (SELECT track_id FROM " + THREAD_TABLE + " WHERE thread_name = ?) ORDER BY timestamp ASC";
    const std::string QUERY_OVERLAP_ANALYSIS_BY_TYPE_DB_SQL =
        "SELECT deviceId as name, startNs - ? as startNs, endNs - ? as endNs, endNs - startNs as duration "
        "FROM " + TABLE_OVERLAP_ANALYSIS + " WHERE type = ? ORDER BY deviceId ASC, startNs ASC";

const std::string QUERY_COMMUNICATION_GROUP_MAP_TEXT_SQL =
    "SELECT pid as groupName, tid as planeId, thread_name as threadName FROM " + THREAD_TABLE + " "
    "WHERE track_id in ( "
    "    SELECT track_id FROM " + THREAD_TABLE + " thread WHERE pid in ( "
    "        SELECT pid FROM " + PROCESS_TABLE + " process WHERE process_name in "
                                                 " ('HCCL', 'COMMUNICATION', 'Communication') "
    "    ) "
    ") ORDER BY thread_sort_index ASC";

const std::string QUERY_COMMUNICATION_OP_BY_GROUP_ID_TEXT_SQL =
    "SELECT id, name, timestamp - ? as startNs, duration, end_time - ? as endNs FROM " + SLICE_TABLE + " "
    "WHERE track_id = ? ORDER by timestamp ASC";
const std::string QUERY_COMMUNICATION_OP_BY_GROUP_ID_DB_SQL =
    "SELECT opId as id, str.value as name, startNs - ? as startNs, endNs - startNs as duration, endNs - ? as endNs "
    "FROM " + TABLE_COMMUNICATION_OP + " op JOIN " + TABLE_STRING_IDS + " str ON op.opName = str.id "
    "WHERE groupName = ? ORDER BY startNs ASC";
const std::string QUERY_COMMUNICATION_GROUP_ID_TEXT_SQL =
    "SELECT track_id as groupId, thread_name as groupName "
    "FROM " + THREAD_TABLE + " WHERE pid in (SELECT pid FROM " + PROCESS_TABLE + " WHERE process_name in "
                                                                 " ('HCCL', 'COMMUNICATION', 'Communication'))";

const std::string QUERY_BYTE_ALIGNMENT_ANALYZER_DATA_SQL = "SELECT name, args FROM " + SLICE_TABLE +
    " WHERE SUBSTR(name, 1, 4) = 'hcom' OR SUBSTR(name, 1, 6) = 'Memcpy' OR SUBSTR(name, 1, 6) = 'Reduce'";

// 兼容老版本（1.0.0）
const std::string QUERY_COMMUNICATION_GROUP_ID_DB_1_0_SQL =
    "SELECT groupId, 'Group ' || row_num || ' Communication' as groupName "
    "FROM ( "
    "    SELECT groupName as groupId, row_number() OVER (ORDER BY groupName ASC) -1 as row_num "
    "    FROM " + TABLE_COMMUNICATION_OP + " GROUP BY groupName "
    ")";
const std::string QUERY_COMMUNICATION_GROUP_ID_DB_SQL =
    "SELECT op.groupName as groupId, 'Group ' || str.value || ' Communication' as groupName "
    "FROM ( "
    "    SELECT groupName FROM " + TABLE_COMMUNICATION_OP + " GROUP BY groupName ORDER BY groupName ASC "
    ") op JOIN " + TABLE_STRING_IDS + " str on op.groupName = str.id";

class TextSqlConstant {
public:
    static std::string GetInsertSliceSql()
    {
        std::string sql = "INSERT INTO slice "
            " (timestamp, duration, name, track_id, cat, args, cname, end_time, flag_id) VALUES"
            " (?,?,?,?,?,?,?,?,?)";
        for (int i = 0; i < CACHE_SIZE - 1; ++i) {
            sql.append(",(?,?,?,?,?,?,?,?,?)");
        }
        return sql;
    }
    static std::string GetInsertFlowSql()
    {
        std::string sql = "INSERT INTO flow (flow_id, name, track_id, timestamp, cat, type)  VALUES (?,?,?,?,?,?)";
        for (int i = 0; i < CACHE_SIZE - 1; ++i) {
            sql.append(",(?,?,?,?,?,?)");
        }
        return sql;
    }
    static std::string GetInsertCounterSql()
    {
        std::string sql = "INSERT INTO  counter  (name, pid, timestamp, cat, args) VALUES (?,?,?,?,?)";
        for (int i = 0; i < CACHE_SIZE - 1; ++i) {
            sql.append(",(?,?,?,?,?)");
        }
        return sql;
    }

    static std::string GetSummarySliceSql(uint64_t size)
    {
        std::string sql = "SELECT timestamp , end_time FROM " + SLICE_TABLE + " WHERE track_id in ( ? ";
        for (uint64_t i = 0; i < size; ++i) {
            sql += ", ? ";
        }
        sql += ") ORDER BY timestamp";
        return sql;
    }
    static std::string GetSearchSliceNameCountSql(bool isMatchExact, bool isMatchCase)
    {
        std::string nameMatch = GetSearchNameSqlSuffix(isMatchExact, isMatchCase);
        std::string sql = "SELECT count(*) FROM " + SLICE_TABLE + " WHERE " + nameMatch;
        return sql;
    }
    static std::string GetSearchSliceNameSql(bool isMatchExact, bool isMatchCase)
    {
        std::string nameMatch = GetSearchNameSqlSuffix(isMatchExact, isMatchCase);
        std::string sql = "SELECT id, pid, tid, timestamp - ? as startTime, duration, track_id AS trackId"
            " FROM " + SLICE_TABLE + " JOIN " + THREAD_TABLE + " USING (track_id) WHERE " + nameMatch +
            " ORDER BY timestamp LIMIT 1 OFFSET ?";
        return sql;
    }
    static std::string GetComputeStatisticsSQL(const std::string &stepId)
    {
        std::string stepCondition;
        if (!stepId.empty() && stepId != "ALL") {
            stepCondition.append(" and step_id =? ");
        }
        std::string sql = "SELECT sum(duration) as duration,accelerator_core as acceleratorCore FROM kernel_detail"
            " WHERE accelerator_core in ('AI_CPU','AI_CORE', 'AI_VECTOR_CORE', 'MIX_AIC', 'MIX_AIV', 'FFTS_PLUS') " +
            stepCondition + " GROUP BY accelerator_core";
        return sql;
    }
    static std::string GetCommunicationStatisticsSql(const std::string &stepId)
    {
        std::string timestampCondition;
        if (!stepId.empty()) {
            timestampCondition = " and timestamp >= ? and timestamp <= ? ";
        }
        std::string sql = "select duration / 1000, t.thread_name as overlapType from (select sum(duration) as duration,"
            " track_id from " + SLICE_TABLE + " where track_id in (select track_id from thread where thread_name "
            " in ('Communication(Not Overlapped)', 'Communication')) " +
            timestampCondition + " group by track_id) s left join thread t on s.track_id=t.track_id";
        return sql;
    }
    static std::string GetQueryLayerDataSql(std::vector<std::string> layers)
    {
        const auto layerStr = StringUtil::Join4SqlGroup(std::move(layers));
        return "SELECT sum(case when name != 'Communication' then duration else 0 end) "
                "AS totalTime, count(distinct name) FROM slice "
                "WHERE lower(name) LIKE lower(?) and slice.track_id IN "
                "( SELECT track_id FROM process JOIN thread t ON process.pid = t.pid "
                " WHERE lower(process_name) in ("+ layerStr +") ) ";
    }
    static std::string GetQueryPythonViewDataSql(const std::string &order, const std::string &orderByField,
        std::vector<std::string> layers)
    {
        std::string orderBy;
        if (order == "descend") {
            orderBy = " ORDER BY " + orderByField + " DESC";
        } else {
            orderBy = " ORDER BY " + orderByField + " ASC";
        }
        const auto layerStr = StringUtil::Join4SqlGroup(std::move(layers));
        std::string sql = "SELECT name, ROUND(cast(sum(duration) as double) * 100 / ?, 2) as "
            "time, sum(duration) / 1000.0 as totalTime, count(1) as numberCalls, "
            "ROUND(avg(duration) / 1000.0, 4) as avg, "
            "min(duration) / 1000.0 as min, max(duration) / 1000.0 as max "
            "FROM slice WHERE lower(name) LIKE lower(?) AND slice.track_id IN ( SELECT track_id "
            "FROM process JOIN thread t ON process.pid = t.pid "
            "WHERE lower(process_name) in (" + layerStr + ")) GROUP BY name " +
            orderBy + " limit ? offset ?";
        return sql;
    }

    static std::string GetAICoreViewDataSql()
    {
        std::string orderBy = " ORDER BY timestamp ASC";
 
        std::string sql = "SELECT timestamp, args "
            "FROM counter WHERE name = 'AI Core Freq' " + orderBy;
        return sql;
    }

    static std::string GetKernelDetailSql(const std::string &order, const std::string &orderByField,
        const std::string &coreType, const std::vector<std::pair<std::string, std::string>>& filters)
    {
        std::string orderBy = " ORDER BY " + orderByField + (order == "descend" ? " DESC" : " ASC");
        std::string coreTypes;
        if (!coreType.empty()) {
            coreTypes = " AND accelerator_core = ? ";
        }
        std::string sql = "SELECT id, task_id as taskId, name, op_type AS type, accelerator_core AS acceleratorCore, "
            "duration, start_time AS startTime, wait_time AS waitTime, block_dim AS blockDim, "
            "input_shapes AS inputShapes, input_data_types AS inputDataTypes, input_formats AS inputFormats, "
            "output_shapes AS outputShapes, output_data_types AS outputDataTypes, "
            "output_formats AS outputFormats FROM kernel_detail WHERE 1=1";
        for (const auto &filter : filters) {
            if (!StringUtil::CheckSqlValid(filter.first)) {
                Server::ServerLog::Error("There is an SQL injection attack on this parameter. param: filter");
                sql.clear();
                return sql;
            }
            sql += " AND lower(" + filter.first + ") LIKE lower(?) ";
        }
        sql += coreTypes + orderBy + " limit ? offset ?";
        return sql;
    }
    static std::string GetThreadSameOperatorsDetailsSql(const std::string &order, const std::string &orderByField,
                                                        const std::vector<std::string> &trackIdList)
    {
        std::string orderBy = " ORDER BY " + orderByField + (order == "descend" ? " DESC" : " ASC");
        std::string trackIdPlaceholders = StringUtil::join(trackIdList, ", ");
        std::string sql = "SELECT timestamp, duration, id, coalesce(depth, 0) as depth, track_id FROM " + SLICE_TABLE +
            " WHERE name = ? AND track_id in (" + trackIdPlaceholders + ") AND timestamp <= ? AND timestamp "
            " + duration >= ? " + orderBy + " limit ? offset ?";
        return sql;
    }

    static std::string GetSearchNameSqlSuffix(bool isMatchExact, bool isMatchCase)
    {
        std::string nameMatch;
        if (isMatchExact && isMatchCase) {
            nameMatch = "name like ?";
        } else if (isMatchExact) {
            nameMatch = "lower(name) like lower(?)";
        } else if (isMatchCase) {
            nameMatch = "name like '%'||?||'%'";
        } else {
            nameMatch = "lower(name) like lower('%'||?||'%')";
        }
        return nameMatch;
    }

    static std::string GetSearchSliceDetailSql(bool isMatchExact, bool isMatchCase, const std::string& order,
        const std::string& orderByField)
    {
        std::string orderBy = " ORDER BY " + orderByField + (order == "descend" ? " DESC" : " ASC");
        std::string nameMatch = GetSearchNameSqlSuffix(isMatchExact, isMatchCase);
        std::string sql = "SELECT s.name as name, s.timestamp as timestamp, s.duration as duration,"
            " s.id as id, t.tid as tid, t.pid as pid FROM " + SLICE_TABLE + " s JOIN " + THREAD_TABLE +
            " t on s.track_id = t.track_id WHERE " + nameMatch + orderBy + " limit ? offset ?";
        return sql;
    }

    static std::string GenerateAclnnQueryTextSql(const Protocol::KernelDetailsParams &params)
    {
        std::string sql =
            "SELECT s.id as id, s.name as name, s.timestamp - ? as startTime, s.duration / 1000 as duration, "
            "t.pid as pid, t.tid as tid, t.track_id as track_id "
            "FROM " + SLICE_TABLE + " s JOIN " + THREAD_TABLE + " t on s.track_id = t.track_id "
            "WHERE t.thread_name LIKE 'Stream%' AND s.name IN ( "
            "    SELECT name FROM " + SLICE_TABLE + "    WHERE name LIKE 'aclnn%' "
            "    GROUP BY name HAVING COUNT(name) >= ? "
            ") ORDER BY " + params.orderBy + " " + params.order;
        return sql;
    }

    static std::string GenerateOperatorDispatchQueryTextSql(const Protocol::KernelDetailsParams &params)
    {
        std::string sql =
            "SELECT s.id AS id, s.name AS name, s.timestamp - ? AS startTime, s.duration / 1000 AS duration, "
            "  t.pid AS pid, t.tid AS tid, t.track_id AS track_id "
            "FROM " + SLICE_TABLE + " s "
            "  JOIN " + THREAD_TABLE + " t ON s.track_id = t.track_id "
            "WHERE "
            "  t.thread_name LIKE 'Thread%' "
            "  AND s.name LIKE '%aclopCompileAndExecute' "
            "ORDER BY " + params.orderBy + " " + params.order;
        return sql;
    }

    static std::string GenerateAICpuQueryTextSql(const std::vector<std::string> &replace,
        const Protocol::KernelDetailsParams &params,
        const std::map<std::string, Timeline::AICpuCheckDataType> &dataTypeMap)
    {
        std::vector<std::string> opTypeList{};
        for (const auto &item : dataTypeMap) { // 获取除other以外的算子类型列表
            if (item.first != "other") {
                opTypeList.emplace_back(item.first);
            }
        }
        std::vector<std::string> dataTypeCheck{};
        for (const auto &item : dataTypeMap) {
            std::string opType = item.first;
            if (item.first == "other") { // 对于other，使用Not IN排除opTypeList以外的算子类型
                opType = StringUtil::Join4SqlGroup(opTypeList);
            }
            dataTypeCheck.emplace_back(GenerateAICpuOpFilterSql(opType, item.second));
        }
        std::string dataTypeCheckSql = StringUtil::join(dataTypeCheck, "OR");

        std::string sql =
            "SELECT kd.name as name, kd.op_type as type, kd.start_time - ? as startTime, kd.duration as duration, "
            "t.pid as pid, t.tid as tid, t.track_id as track_id, s.id as id, "
            "lower(kd.input_data_types) as input,  lower(kd.output_data_types) as output "
            "FROM " + KERNEL_DETAIL + " kd "
            "JOIN " + SLICE_TABLE +" s ON kd.name = s.name AND kd.start_time = s.timestamp "
            "JOIN " + THREAD_TABLE + " t ON s.track_id = t.track_id "
            "WHERE kd.accelerator_core='AI_CPU' AND ("
            "    lower(kd.op_type) IN (" +
            StringUtil::Join4SqlGroup(replace) +
            ") " // 特定类型的算子可以修改代码
            "    OR ("
            "    " +
            dataTypeCheckSql + // 检查数据类型是否符合要求
            "    ) OR "
            "    kd.duration >= ?" // 执行时间超过20us
            ") ORDER BY " +
            params.orderBy + " " + params.order;
        return sql;
    }

    static std::string GenerateAICpuQueryDbSql(const std::vector<std::string> &replace,
        const Protocol::KernelDetailsParams &params,
        const std::map<std::string, Timeline::AICpuCheckDataType> &dataTypeMap)
    {
        std::vector<std::string> opTypeList{};
        for (const auto &item : dataTypeMap) { // 获取除other以外的算子类型列表
            if (item.first != "other") {
                opTypeList.emplace_back(item.first);
            }
        }
        std::vector<std::string> dataTypeCheck{};
        for (const auto &item : dataTypeMap) {
            std::string opType = item.first;
            if (item.first == "other") { // 对于other，使用Not IN排除opTypeList以外的算子类型
                opType = StringUtil::Join4SqlGroup(opTypeList);
            }
            dataTypeCheck.emplace_back(GenerateAICpuOpFilterSqlDB(opType, item.second));
        }
        std::string dataTypeCheckSql = StringUtil::join(dataTypeCheck, "OR");

        std::string sql = "SELECT info.ROWID as id, s2.value as name, s1.value as type, s0.value as unit, "
            "t.startNs - ? as startTime, (t.endNs - t.startNs) / 1000 as duration, 'Ascend Hardware' as pid, "
            "t.streamId as tid, t.depth as depth, lower(s3.value) as input, lower(s4.value) as output "
            "FROM COMPUTE_TASK_INFO info "
            "JOIN STRING_IDS s0 ON info.taskType = s0.id "
            "JOIN TASK t ON info.globalTaskId = t.globalTaskId "
            "JOIN STRING_IDS s1 ON info.opType = s1.id "
            "JOIN STRING_IDS s2 ON info.name = s2.id "
            "JOIN STRING_IDS s3 ON info.inputDataTypes = s3.id "
            "JOIN STRING_IDS s4 ON info.outputDataTypes = s4.id "
            "WHERE s0.value ='AI_CPU' AND ("
            "    lower(s1.value) IN (" +  StringUtil::Join4SqlGroup(replace) +
            ") " // 特定类型的算子可以修改代码
            "    OR ("
            "    " +
            dataTypeCheckSql + // 检查数据类型是否符合要求
            "    ) OR "
            "    duration >= ?" // 执行时间超过20us
            ") ORDER BY " + params.orderBy + " " + params.order;
        return sql;
    }

    static std::string GenerateFuseableOpFilterTextSql(const Protocol::KernelDetailsParams &params,
        const Timeline::FuseableOpRule &rule)
    {
        std::string sql = "WITH data AS ( "
            "SELECT kd.rank_id, kd.name as name, kd.op_type, kd.accelerator_core, kd.start_time - ? as startTime, "
            "s.duration / 1000 as duration, t.pid as pid, t.tid as tid, s.id as id, s.track_id as track_id, "
            "ROW_NUMBER() OVER (ORDER BY s.track_id ASC, s.timestamp ASC) AS row_num "
            "FROM " +
            KERNEL_DETAIL +
            " kd "
            "JOIN " +
            SLICE_TABLE +
            " s ON kd.name = s.name AND kd.start_time = s.timestamp "
            "JOIN " +
            THREAD_TABLE +
            " t ON s.track_id = t.track_id "
            "WHERE kd.accelerator_core NOT IN ('HCCL', 'COMMUNICATION') ) "
            "SELECT d0.* FROM data d0 ";
        for (size_t i = 1; i < rule.opList.size(); ++i) { // 上文保证rule.opList.size() ≥ 2
            std::string table = "d" + std::to_string(i);
            sql += "JOIN data " + table + " ON " + table + ".row_num = d0.row_num + " + std::to_string(i) + " AND " +
                table + ".op_type = '" + rule.opList.at(i) + "' ";
        }
        sql += "WHERE d0.op_type = '" + rule.opList.at(0) + "' ORDER BY " + params.orderBy + " " + params.order;
        return sql;
    }

    static std::string GenerateFuseableOpFilterDbSql(const Protocol::KernelDetailsParams &params,
        const Timeline::FuseableOpRule &rule)
    {
        std::string sql = "WITH data AS ( "
            "SELECT info.ROWID as id, task.deviceId as deviceId, s1.value as name, s2.value as op_type, task.taskType, "
            "task.startNs - ? as startTime, (task.endNs - task.startNs) / 1000 as duration, 'Ascend Hardware' as pid, "
            "task.streamId as tid, task.depth as depth, "
            "ROW_NUMBER() OVER (ORDER BY task.globalPid ASC, task.startNs ASC) AS row_num "
            "FROM " + TABLE_COMPUTE_TASK_INFO + " info "
            "JOIN " + TABLE_TASK + " task ON info.globalTaskId = task.globalTaskId "
            "JOIN " + TABLE_STRING_IDS + " s1 ON info.name = s1.id "
            "JOIN " + TABLE_STRING_IDS + " s2 ON info.opType = s2.id ) "
            "SELECT d0.* FROM data d0 ";
        for (size_t i = 1; i < rule.opList.size(); ++i) { // 上文保证rule.opList.size() ≥ 2
            std::string table = "d" + std::to_string(i);
            sql += "JOIN data " + table + " ON " + table + ".row_num = d0.row_num + " + std::to_string(i) +
                   " AND " + table + ".op_type = '" + rule.opList.at(i) + "' ";
        }
        sql += "WHERE d0.op_type = '" +  rule.opList.at(0) + "' ORDER BY " + params.orderBy + " " + params.order;
        return sql;
    }

    static std::string QueryAffinityOptimizerTextSql(const std::string &optimizers, const std::string &orderBy,
                                                     const std::string &order)
    {
        std::string sql = "Select (s.timestamp - ?) as startTime, (s.duration / 1000) as duration, s.name as name, "
            "t.pid as pid, t.tid as tid, s.id as id, t.track_id as track_id "
            "From " + SLICE_TABLE + " s Join " + THREAD_TABLE + " t ON s.track_id = t.track_id "
            "WHERE s.name IN ( " + optimizers + ") order by " + orderBy + " " + order;
        return sql;
    }

    static std::string QueryAffinityOptimizerDbSql(const std::string &optimizers, const std::string &orderBy,
                                                     const std::string &order)
    {
        std::string sql =
            "SELECT py.ROWID as id, py.startNs - ? as startTime, (py.endNs - py.startNs) / 1000 as duration, "
            "str.value as originOptimizer, py.globalTid as pid, 'pytorch' as tid, py.depth as depth "
            "FROM " + TABLE_STRING_IDS + " str JOIN " + TABLE_API + " py ON py.name = str.id "
            "WHERE str.value IN (" + optimizers + ") ORDER BY " + orderBy + " " + order;
        return sql;
    }

private:
    static std::string GenerateAICpuOpFilterSql(const std::string &opType, const Timeline::AICpuCheckDataType &dataType)
    {
        std::string sql = " ( ";
        if (std::find(opType.begin(), opType.end(), ',') == opType.end()) { // 输入为单个算子类型
            sql += "lower(kd.op_type) = '" + opType + "' AND ";
        } else { // 输入为算子类型组
            sql += "lower(kd.op_type) NOT IN ( " + opType + " ) AND ";
        }
        sql += "lower(kd.input_data_types) NOT IN ( " + StringUtil::Join4SqlGroup(dataType.input) + " ) AND "
            "lower(kd.output_data_types) NOT IN ( " + StringUtil::Join4SqlGroup(dataType.output) + " )) ";
        return sql;
    }

    static std::string GenerateAICpuOpFilterSqlDB(const std::string &opType,
        const Timeline::AICpuCheckDataType &dataType)
    {
        std::string sql = " ( ";
        if (std::find(opType.begin(), opType.end(), ',') == opType.end()) { // 输入为单个算子类型
            sql += "lower(s1.value) = '" + opType + "' AND ";
        } else { // 输入为算子类型组
            sql += "lower(s1.value) NOT IN ( " + opType + " ) AND ";
        }
        sql += "lower(s3.value) NOT IN ( " + StringUtil::Join4SqlGroup(dataType.input) + " ) AND "
            "lower(s4.value) NOT IN ( " + StringUtil::Join4SqlGroup(dataType.output) + " )) ";
        return sql;
    }
};
}
// LCOV_EXCL_BR_STOP

#endif // PROFILER_SERVER_TEXTSQLCONSTANT_H
