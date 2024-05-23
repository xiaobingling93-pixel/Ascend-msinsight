/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_JSONSQLCONSTANT_H
#define PROFILER_SERVER_JSONSQLCONSTANT_H
#include <string>
#include "StringUtil.h"

namespace Dic::Module::Timeline {
const int CACHE_SIZE = 1000;
const std::string SLICE_TABLE = "slice";
const std::string THREAD_TABLE = "thread";
const std::string PROCESS_TABLE = "process";
const std::string FLOW_TABLE = "flow";
const std::string COUNTER_TABLE = "counter";
const std::string TRACKID_TIME_INDEX = "track_id_timestamp_end_time_index";
const std::string FLOW_INDEX = "flow_id_time_index";
const std::string KERNEL_DETAIL = "kernel_detail";

const std::string UPDATE_PROCESS_NAME_SQL = "INSERT INTO  process  (pid, process_name) VALUES (?, ?) ON CONFLICT (pid) "
    "DO UPDATE SET process_name = excluded.process_name;";
const std::string UPDATE_PROCESS_LABLE_SQL =
    "INSERT INTO process (pid, label) VALUES (?, ?) ON CONFLICT (pid) DO UPDATE SET label = excluded.label;";
const std::string UPDATE_PROCESS_SORTINDEX_SQL = "INSERT INTO process (pid, process_sort_index) VALUES (?, ?) ON "
    "CONFLICT (pid) DO UPDATE SET process_sort_index = "
    "excluded.process_sort_index;";
const std::string UPDATE_THREAD_INFO_SQL = "INSERT INTO thread (track_id, tid, pid) VALUES (?, ?, ?) ON CONFLICT "
    "(track_id) DO UPDATE SET tid = excluded.tid, pid = excluded.pid;";
const std::string UPDATE_THREAD_NAME_SQL = "INSERT INTO thread (track_id, tid, pid, thread_name) VALUES (?, ?, ?, "
    "?) ON CONFLICT (track_id) DO UPDATE SET tid "
    "= excluded.tid, pid = excluded.pid, thread_name = excluded.thread_name;";
const std::string UPDATE_THREAD_SORTINDEX_SQL = "INSERT INTO thread (track_id, thread_sort_index) VALUES (?, ?) ON "
    "CONFLICT (track_id) DO UPDATE SET thread_sort_index = "
    "excluded.thread_sort_index;";
const std::string SIMULATION_UPDATE_THREAD_NAME_SQL = "INSERT INTO thread"
    " (track_id, tid, pid, thread_name, thread_sort_index) VALUES (?, ?, ?, ?, ?)"
    " ON CONFLICT (track_id) DO UPDATE "
    " SET tid = excluded.tid, pid = excluded.pid, thread_name = excluded.thread_name;";
const std::string CREATE_TABLE_SQL = "CREATE TABLE " + SLICE_TABLE +
    " (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp INTEGER, duration INTEGER,"
    " name TEXT, depth INTEGER, track_id INTEGER, cat TEXT, args TEXT, cname TEXT, end_time INTEGER);" +
    "CREATE TABLE " + THREAD_TABLE + " (track_id INTEGER PRIMARY KEY, tid TEXT, pid TEXT, thread_name TEXT," +
    " thread_sort_index INTEGER);" + "CREATE TABLE " + PROCESS_TABLE +
    " (pid TEXT PRIMARY KEY, process_name TEXT, label TEXT," + " process_sort_index INTEGER);" + "CREATE TABLE " +
    FLOW_TABLE + " (id INTEGER PRIMARY KEY AUTOINCREMENT, flow_id TEXT, name TEXT, cat TEXT," +
    " track_id INTEGER, timestamp INTEGER, type TEXT);" + "CREATE TABLE " + COUNTER_TABLE +
    " (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, pid TEXT," + "timestamp INTEGER, cat TEXT, args TEXT);";

const std::string CREATE_INDEX_SQL = "CREATE INDEX " + TRACKID_TIME_INDEX + " ON " + SLICE_TABLE +
    " (track_id, timestamp, end_time);" + "CREATE INDEX " + FLOW_INDEX + " ON " + FLOW_TABLE + " (cat);";
const std::string QUERY_SLICE_BY_TRACKID_SQL =
    "select id, timestamp, end_time as endTime from slice where track_id = ? order by timestamp;";
const std::string QUERY_ALL_TRACKID_SQL = "select track_id as trackId from thread;";
const std::string QUERY_ALL_SLICE_IN_RANGE_BY_TRACKID_SQL =
    "SELECT id, timestamp, end_time FROM " + SLICE_TABLE + " WHERE track_id = ? ";
const std::string QUERY_UINT_THREAD_SQL = "SELECT id, timestamp, duration, end_time AS endTime, name"
    " FROM " +
    SLICE_TABLE + " WHERE track_id = ? AND timestamp <= ? AND endTime >= ?";
const std::string QUERY_EXTREMETIME_OF_FIRST_DEPTH_SQL =
    "SELECT min(timestamp) as minTimestamp, max(end_time) AS maxTimestamp"
    " FROM " +
    SLICE_TABLE + " WHERE track_id = ? AND timestamp <= ? AND end_time >= ? ;";
const std::string QUERY_SLICE_DETAIL_SQL = "SELECT id, timestamp, duration, name, track_id, cat, args"
    " FROM " +
    SLICE_TABLE + " WHERE id = ? AND track_id = ? AND abs(timestamp - ?) <= 500 Order by duration DESC";
const std::string QUERY_DURATION_FROM_SLICE_BY_TIME_RANGE_SQL = "SELECT id, timestamp, duration FROM " + SLICE_TABLE +
    " WHERE end_time <= ? AND timestamp >= ? AND track_id = ? Order by timestamp";
const std::string QUERY_KERNAL_SHAPE_SQL =
    "SELECT accelerator_core, input_shapes AS inputShapes, input_data_types AS inputDataTypes, "
    "input_formats AS inputFormats, output_shapes AS outputShapes, "
    "output_data_types AS outputDataTypes, output_formats AS outputFormats "
    "FROM " +
    KERNEL_DETAIL + " WHERE name = ? AND start_time = ?";
const std::string QUERY_FLOW_BY_FLOWID_SQL = "SELECT name, cat, flow_id as flowId, timestamp, type, track_id as trackId"
    " FROM " +
    FLOW_TABLE + " WHERE flow_id = ?";
const std::string QUERY_ALL_THREAD_SQL = "SELECT track_id as trackId, tid, pid"
    " FROM " +
    THREAD_TABLE + " ;";
const std::string QUERY_FLOW_BY_TIME_RANGE_SQL = "SELECT name, flow_id as flowId, type, timestamp"
    " FROM " +
    FLOW_TABLE + " WHERE timestamp >= ? AND timestamp <= ? AND track_id = ? GROUP BY flowId";
const std::string QUERY_SLICE_BY_TIME_RANGE_SQL = "SELECT timestamp, end_time as endTime"
    " FROM " +
    SLICE_TABLE + " WHERE timestamp >= ? AND endTime <= ? AND track_id = ? order by timestamp";
const std::string QUERY_SLICE_BY_TIME_POINT_SQL = "SELECT id, timestamp, end_time as endTime, name"
    " FROM " +
    SLICE_TABLE + " WHERE timestamp <= ? AND endTime >= ? AND track_id = ?;";
const std::string QUERY_UNITS_META_SQL =
    " SELECT pt.pid, pt.process_name AS processName, pt.label, pt.tid, pt.thread_name AS threadName, "
    "pt.name, pt.args, pt.track_id as trackId "
    " FROM (SELECT p.pid, CASE WHEN p.process_name IS NULL THEN 'Process ' || p.pid ELSE p.process_name END AS "
    " process_name,p.label,p.process_sort_index,t.tid,t.thread_name,t.track_id,t.thread_sort_index,c.name,c.args "
    " FROM " +
    PROCESS_TABLE + " p LEFT JOIN  " + THREAD_TABLE + " t ON p.pid = t.pid LEFT JOIN ( SELECT pid, name, args FROM " +
    COUNTER_TABLE +
    " GROUP BY "
    " name, pid ) c ON c.pid = p.pid ) AS pt WHERE pt.process_name IS NOT NULL "
    " ORDER BY pt.process_sort_index ASC, pt.thread_sort_index ASC, pt.name ASC;";
const std::string QUERY_EXETREME_TIME_SQL = "SELECT  min(minTimestamp) AS totalMinTimestamp, max(maxTimestamp) AS "
    "totalMaxTimestamp FROM (SELECT min(timestamp) "
    "as minTimestamp, max(timestamp) as maxTimestamp FROM " +
    SLICE_TABLE + " UNION SELECT min(timestamp) as minTimestamp, max(timestamp) as maxTimestamp FROM " + COUNTER_TABLE +
    ")";
const std::string QUERY_FLOWCATEGORY_EVENTS_FAST_SQL =
    "select id,track_id AS trackId,timestamp,flow_id AS flowId ,type from " + FLOW_TABLE +
    " WHERE cat = ? ORDER BY trackId, timestamp;";
const std::string QUERY_UNIT_COUNTER_SQL = "SELECT timestamp - ? as startTime, args"
    " FROM " +
    COUNTER_TABLE +
    " WHERE pid = ? AND name = ?"
    " AND startTime >= ? AND startTime <= ? ORDER BY timestamp ASC";
const std::string QUERY_LAYER_DATA_SQL = "SELECT sum(duration) AS totalTime, count(distinct name) FROM slice "
    "WHERE lower(name) LIKE lower(?) and slice.track_id IN "
    "( SELECT track_id FROM process JOIN thread t ON process.pid = t.pid WHERE process_name = ? ) ";
const std::string QUERY_QUERY_TYPE_SQL =
    "SELECT DISTINCT accelerator_core FROM " + KERNEL_DETAIL + " ORDER BY accelerator_core";

const std::string QUERY_ACLNN_OP_CNT_EXCEED_THRESHOLD_SQL =
    "SELECT s.name as name, s.timestamp - ? as startTime, s.duration as duration, t.pid as pid, t.tid as tid "
    "FROM " + SLICE_TABLE + " s JOIN " + THREAD_TABLE + " t on s.track_id = t.track_id "
    "WHERE s.name IN ( "
    "    SELECT name FROM " + SLICE_TABLE + " WHERE name LIKE 'AscendCL@aclnn%' AND name NOT LIKE '%GetWorkspaceSize' "
    "    GROUP BY name HAVING COUNT(name) >= ? )";
const std::string QUERY_AFFINITY_API_SQL =
    "SELECT s.track_id as track, s.id as id, s.name as name, s.timestamp - ? as startTime, s.duration as duration, "
    "t.pid as pid, t.tid as tid FROM " + SLICE_TABLE + " s "
    "JOIN " + THREAD_TABLE + " t on s.track_id = t.track_id "
    "WHERE s.name LIKE 'aten::%' OR s.name LIKE 'npu::%' ORDER BY s.track_id ASC, s.timestamp ASC";

const std::string QUERY_FUSEABLE_OP_SUB_SQL = "WITH data AS ( "
    "SELECT kd.rank_id, kd.name, kd.op_type, kd.accelerator_core, kd.start_time, kd.duration, t.pid, t.tid, "
    "ROW_NUMBER() OVER (ORDER BY s.track_id ASC, s.timestamp ASC) AS row_num FROM " + KERNEL_DETAIL + " kd "
    "JOIN " + SLICE_TABLE + " s ON kd.name = s.name AND kd.start_time = s.timestamp "
    "JOIN " + THREAD_TABLE + " t ON s.track_id = t.track_id "
    "WHERE kd.accelerator_core != 'HCCL' ) "; // 过滤去kernel_detail中所有非HCCL算子，并按照track和start_time联合排序

class JsonSqlConstant {
public:
    static std::string GetInsertSliceSql()
    {
        std::string sql = "INSERT INTO slice "
            " (timestamp, duration, name, track_id, cat, args, cname, end_time) VALUES"
            " (?,?,?,?,?,?,?,?)";
        for (int i = 0; i < CACHE_SIZE - 1; ++i) {
            sql.append(",(?,?,?,?,?,?,?,?)");
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
    static std::string GetInsertCounterql()
    {
        std::string sql = "INSERT INTO  counter  (name, pid, timestamp, cat, args) VALUES (?,?,?,?,?)";
        for (int i = 0; i < CACHE_SIZE - 1; ++i) {
            sql.append(",(?,?,?,?,?)");
        }
        return sql;
    }
    static std::string GetSliceByIdListSql(uint64_t idSetSize)
    {
        std::string sliceSql = "SELECT id, timestamp, end_time, name, cname from slice where id in ( ? ";
        for (int i = 0; i < idSetSize - 1; ++i) {
            sliceSql += ", ? ";
        }
        sliceSql += " , ? );";
        return sliceSql;
    }

    static std::string GetSummarySliceSql(uint64_t size)
    {
        std::string sql = "SELECT timestamp , end_time FROM " + SLICE_TABLE + " WHERE track_id in ( ? ";
        for (int i = 0; i < size; ++i) {
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
            " FROM " +
            SLICE_TABLE + " JOIN " + THREAD_TABLE + " USING (track_id) WHERE " + nameMatch +
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
        if (stepId.empty()) {
            timestampCondition = " and timestamp >= ? and timestamp <= ? ";
        }
        std::string sql = "select duration / 1000, t.thread_name as overlapType from (select sum(duration) as duration,"
            " track_id from " +
            SLICE_TABLE +
            " where track_id in (select track_id from thread where thread_name "
            " in ('Communication(Not Overlapped)', 'Communication')) " +
            timestampCondition + " group by track_id) s left join thread t on s.track_id=t.track_id";
        return sql;
    }
    static std::string GetQueryPythonViewDataSql(const std::string &order, const std::string &orderByField)
    {
        std::string orderBy;
        if (order == "descend") {
            orderBy = " ORDER BY " + orderByField + " DESC";
        } else {
            orderBy = " ORDER BY " + orderByField + " ASC";
        }
        std::string sql = "SELECT name, ROUND(cast(sum(duration) as double) * 100 / ?, 2) as "
            "time, sum(duration) / 1000.0 as totalTime, count(1) as numberCalls, "
            "ROUND(avg(duration) / 1000.0, 4) as avg, "
            "min(duration) / 1000.0 as min, max(duration) / 1000.0 as max "
            "FROM slice WHERE lower(name) LIKE lower(?) AND slice.track_id IN ( SELECT track_id "
            "FROM process JOIN thread t ON process.pid = t.pid "
            "WHERE process_name = ? ) GROUP BY name " +
            orderBy + " limit ? offset ?";
        return sql;
    }
    static std::string GetKernelDetailSql(const std::string &order, const std::string &orderByField,
        const std::string &coreType)
    {
        std::string orderBy;
        std::string coreTypes;
        if (order == "descend") {
            orderBy = " ORDER BY " + orderByField + " DESC";
        } else {
            orderBy = " ORDER BY " + orderByField + " ASC";
        }
        if (!coreType.empty()) {
            coreTypes = " AND accelerator_core = ? ";
        }
        std::string sql = "SELECT name, op_type as type, accelerator_core AS acceleratorCore, start_time AS startTime, "
            "duration, wait_time as waitTime, block_dim AS blockDim, input_shapes AS inputShapes, "
            "input_data_types AS inputDataTypes, input_formats AS inputFormats, "
            "output_shapes AS outputShapes, output_data_types AS outputDataTypes, "
            "output_formats AS outputFormats FROM kernel_detail "
            "where 1=1 and lower(name) LIKE lower(?) " +
            coreTypes + orderBy + " limit ? offset ?";
        return sql;
    }
    static std::string GetThreadSameOperatorsDetailsSql(const std::string &order, const std::string &orderByField)
    {
        std::string orderBy;
        if (order == "descend") {
            orderBy = " ORDER BY " + orderByField + " DESC";
        } else {
            orderBy = " ORDER BY " + orderByField + " ASC";
        }
        std::string sql = "SELECT timestamp, duration, id, coalesce(depth, 0) as depth FROM " + SLICE_TABLE +
            " WHERE name = ? AND track_id = ? AND timestamp <= ? AND timestamp + duration >= ? " + orderBy +
            " limit ? offset ?";
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

    static std::string GetSearchSliceDetailSql(bool isMatchExact, bool isMatchCase, std::string order,
        std::string orderByField)
    {
        std::string orderBy;
        if (order == "descend") {
            orderBy = " ORDER BY " + orderByField + " DESC";
        } else {
            orderBy = " ORDER BY " + orderByField + " ASC";
        }
        std::string nameMatch = GetSearchNameSqlSuffix(isMatchExact, isMatchCase);
        std::string sql = "SELECT name, timestamp, duration FROM " + SLICE_TABLE + " WHERE " + nameMatch + orderBy +
            " limit ? offset ?";
        return sql;
    }

    static std::string GenerateAICpuQuerySql(const std::vector<std::string> &replace,
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
            "t.pid as pid, t.tid as tid "
            "FROM " + KERNEL_DETAIL + " kd "
            "JOIN " + SLICE_TABLE + " s ON kd.name = s.name AND kd.start_time = s.timestamp "
            "JOIN " + THREAD_TABLE + " t ON s.track_id = t.track_id "
            "WHERE kd.accelerator_core='AI_CPU' AND ("
            "    lower(kd.op_type) IN (" + StringUtil::Join4SqlGroup(replace) + ") " // 特定类型的算子可以修改代码
            "    OR ("
            "    " + dataTypeCheckSql + // 检查数据类型是否符合要求
            "    ) OR "
            "    kd.duration >= ?" // 执行时间超过20us
            ") ";
        return sql;
    }

    static std::string GenerateAICpuQuerySqlDB(const std::vector<std::string> &replace,
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

        std::string sql =
            "SELECT s2.value as name, s1.value as type, s0.value as unit, t.startNs - ? as startTime, "
            "t.endNs - t.startNs as duration, t.globalPid as pid "
            "FROM COMPUTE_TASK_INFO info "
            "JOIN STRING_IDS s0 ON info.taskType = s0.id "
            "JOIN TASK t ON info.globalTaskId = t.globalTaskId "
            "JOIN STRING_IDS s1 ON info.opType = s1.id "
            "JOIN STRING_IDS s2 ON info.name = s2.id "
            "JOIN STRING_IDS s3 ON info.inputDataTypes = s3.id "
            "JOIN STRING_IDS s4 ON info.outputDataTypes = s4.id "
            "WHERE s0.value ='AI_CPU' AND ("
            "    lower(s1.value) IN (" + StringUtil::Join4SqlGroup(replace) + ") " // 特定类型的算子可以修改代码
            "    OR ("
            "    " + dataTypeCheckSql + // 检查数据类型是否符合要求
            "    ) OR "
            "    duration >= ?" // 执行时间超过20us
            ") ";
        return sql;
    }

    static std::string GenerateFuseableOpFilterSql(const Timeline::FuseableOpRule &rule)
    {
        std::string sql = "WITH data AS ( "
            "SELECT kd.rank_id, kd.name, kd.op_type, kd.accelerator_core, kd.start_time - ?, s.duration, "
            "t.pid, t.tid, ROW_NUMBER() OVER (ORDER BY s.track_id ASC, s.timestamp ASC) AS row_num "
            "FROM " + KERNEL_DETAIL + " kd "
            "JOIN " + SLICE_TABLE + " s ON kd.name = s.name AND kd.start_time = s.timestamp "
            "JOIN " + THREAD_TABLE + " t ON s.track_id = t.track_id "
            "WHERE kd.accelerator_core != 'HCCL' ) "
            "SELECT d0.* FROM data d0 ";
        for (int i = 1; i < rule.opList.size(); ++i) { // 上文保证rule.opList.size() ≥ 2
            std::string table = "d" + std::to_string(i);
            sql += "JOIN data " + table + " ON " + table + ".row_num = d0.row_num + " + std::to_string(i) +
                    " AND " + table + ".name = '" + rule.opList.at(i) + "' ";
        }
        sql += "WHERE d0.name = '" +  rule.opList.at(0) + "'";
        return sql;
    }

private:
    static std::string GenerateAICpuOpFilterSql(const std::string& opType, const Timeline::AICpuCheckDataType& dataType)
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

    static std::string GenerateAICpuOpFilterSqlDB(const std::string& opType,
        const Timeline::AICpuCheckDataType& dataType)
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


#endif // PROFILER_SERVER_JSONSQLCONSTANT_H
