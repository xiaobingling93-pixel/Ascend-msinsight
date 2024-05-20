/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_JSONSQLCONSTANT_H
#define PROFILER_SERVER_JSONSQLCONSTANT_H
#include <string>

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

const std::string QUERY_AICPU_OP_EXCEED_THRESHOLD_SQL =
        "SELECT s.name as name, kd.op_type as type, s.timestamp - ? as startTime, s.duration as duration, "
        "t.pid as pid, t.tid as tid FROM ( "
        "    SELECT name, timestamp, duration, track_id FROM " + SLICE_TABLE +
        "    WHERE args LIKE '%Task Type%AI_CPU%' AND duration > ?) s "
        "JOIN " + KERNEL_DETAIL + " kd ON s.name = kd.name AND s.timestamp = kd.start_time "
        "JOIN " + THREAD_TABLE + " t on s.track_id = t.track_id";

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
        std::string sql = "SELECT timestamp, duration FROM " + SLICE_TABLE +
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
};
}


#endif // PROFILER_SERVER_JSONSQLCONSTANT_H
