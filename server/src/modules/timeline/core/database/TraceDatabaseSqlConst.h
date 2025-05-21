/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACEDATABASESQLCONST_H
#define PROFILER_SERVER_TRACEDATABASESQLCONST_H

#include <string>
namespace Dic::Module::Timeline {
const std::string TEMP_FWDBWD_FLOW_TABLE = "tmpFwdbwdFlow";
const std::string CREATE_TEMP_FWDBWD_FLOW_TABLE_TEXT_SQL =
    "DROP TABLE IF EXISTS tmpFwdbwdFlow;"
    "CREATE TEMPORARY TABLE tmpFwdbwdFlow AS "
    "WITH combined AS ( \n"
    "    SELECT f.flow_id, f.type, f.timestamp AS slice_begin, s.end_time AS slice_end \n"
    "    FROM flow f JOIN slice s ON f.track_id = s.track_id AND f.timestamp = s.timestamp \n"
    "    WHERE f.cat = 'fwdbwd' AND f.type IN ('s', 'f') \n"
    "    ORDER by f.flow_id \n"
    "), " // 过滤flow表里前反向数据，并与slice表join，以获取每个元素的结束时间
    "fwd AS ( \n"
    "    SELECT flow_id, slice_begin, slice_end FROM combined WHERE type = 's' \n"
    "), \n"
    "bwd AS ( \n"
    "    SELECT flow_id, slice_begin, slice_end FROM combined WHERE type = 'f' \n"
    "), \n"
    "flowsAscByFwd AS ( \n"
    "    SELECT fwd.slice_begin AS fwdStart, fwd.slice_end AS fwdEnd, \n"
    "    bwd.slice_begin AS bwdStart, bwd.slice_end AS bwdEnd, \n"
    "    ROW_NUMBER() OVER (ORDER BY fwd.slice_begin) AS rowNum \n"
    "    FROM fwd JOIN bwd ON fwd.flow_id = bwd.flow_id \n"
    ") " // 将前反向联系起来，并按前向起始时间升序排列，以方便后续识别前反向边界
    "SELECT fwdStart, fwdEnd, bwdStart, bwdEnd, rowNum FROM flowsAscByFwd;";

const std::string CREATE_TEMP_FWDBWD_FLOW_TABLE_DB_SQL =
    "DROP TABLE IF EXISTS tmpFwdbwdFlow;"
    "CREATE TEMPORARY TABLE tmpFwdbwdFlow AS "
    "with type as ( \n"
    "    SELECT id FROM ENUM_API_TYPE WHERE name = 'op' \n"
    "), " // 过滤cpu_op对应的类型值
    "flowCat as ( \n"
    "    SELECT connectionId \n"
    "    FROM connectionCats WHERE cat = 'fwdbwd' ORDER BY connectionId \n"
    "), " // 过滤所有的前反向连线id
    "flowTable as ( \n"
    "    SELECT ids.id as flowId, ids.connectionId as connectionId \n"
    "    FROM flowCat cats JOIN CONNECTION_IDS ids \n"
    "    ON cats.connectionId = ids.connectionId ORDER by ids.id ASC \n"
    "), " // 过滤前反向连线flowId和connectId数据
    "apiTable as ( \n"
    "    SELECT CAST(startNs AS INTEGER) AS startNs, CAST(endNs AS INTEGER) AS endNs, connectionId FROM PYTORCH_API \n"
    "    WHERE connectionId IS NOT NULL AND type in type \n"
    "    ORDER BY connectionId \n"
    "), " // 过滤出所有的cpu_op数据
    "combined as ( \n"
    "    SELECT startNs, endNs, flow.connectionId FROM flowTable flow join apiTable api \n"
    "    ON flow.flowId = api.connectionId ORDER BY flow.connectionId ASC \n"
    "), " // 建立起连线与界面元素起始时间的联系
    "flowsAscByFwd as (\n"
    "    SELECT s.startNs as fwdStart, s.endNs as fwdEnd, f.startNs as bwdStart, f.endNs as bwdEnd, \n"
    "    ROW_NUMBER() OVER (ORDER BY s.startNs) AS rowNum \n"
    "    FROM combined s JOIN combined f \n"
    "    ON s.connectionId = f.connectionId AND s.startNs < f.startNs \n"
    ") " // 将前反向联系起来，并按前向起始时间升序排列，以方便后续识别前反向边界
    "SELECT fwdStart, fwdEnd, bwdStart, bwdEnd, rowNum FROM flowsAscByFwd;";

const std::string QUERY_FWDBWD_FLOW_DATA_SQL =
    "WITH increaseEndIndex AS ( \n"
    "    SELECT CASE WHEN d1.rowNum != 1 THEN d1.rowNum END as endIndex \n"
    "    FROM tmpFwdbwdFlow d1 LEFT JOIN tmpFwdbwdFlow d2 ON d2.rowNum = d1.rowNum + 1\n"
    "    WHERE d2.bwdStart > d1.bwdStart OR d2.bwdStart IS NULL OR d1.rowNum = 1\n"
    "), " // 按前向开始时间升序处理，前向递增，反向递减，前反向边界，反向有个增大的突变(vpp场景下，vpp的边界是例外，后续需要特殊处理)
    "flowsAscByBwd AS ( \n"
    "    SELECT fwdStart, bwdStart, rowNum as oldRowNum, \n"
    "    ROW_NUMBER() OVER (ORDER BY bwdStart) AS rowNum \n"
    "    FROM tmpFwdbwdFlow \n"
    "), " // 按反向起始时间升序排列，以方便后续识别前反向边界
    "decreaseEndIndex AS ( \n"
    "    SELECT d1.oldRowNum as endIndex \n"
    "    FROM flowsAscByBwd d1 LEFT JOIN flowsAscByBwd d2 ON d2.rowNum = d1.rowNum - 1 \n"
    "    WHERE d1.fwdStart > d2.fwdStart OR d2.fwdStart IS NULL OR d1.rowNum = 1 \n"
    "), " // 按反向开始时间升序排列，过滤出潜在的突变点，作为正向处理的补充，以解决vpp场景下
    "possibleIndex AS ( \n"
    "    SELECT * FROM increaseEndIndex WHERE endIndex is NOT NULL\n"
    "    UNION \n"
    "    SELECT * FROM decreaseEndIndex WHERE endIndex is NOT NULL\n"
    "    ORDER BY endIndex \n"
    "), " // 将前向开始时间升序识别的边界与后向开始时间升序识别的边界进行合并
    "possibleData AS ( \n"
    "    SELECT \n"
    "        CASE WHEN d1.rowNum != 1 THEN d1.fwdEnd ELSE 0 END as nextFpEnd, \n"
    "        CASE WHEN d1.rowNum != 1 THEN d1.bwdStart ELSE 0 END as nextBpStart, \n"
    "        COALESCE(d2.fwdStart, 0) AS prevFpStart, \n"
    "        COALESCE(d2.bwdEnd, 0) AS prevBpEnd, \n"
    "        ROW_NUMBER() OVER (ORDER BY d1.rowNum) AS rowNum \n"
    "    FROM tmpFwdbwdFlow d1 LEFT JOIN tmpFwdbwdFlow d2 ON d2.rowNum = d1.rowNum + 1 \n"
    "    WHERE d1.rowNum = 1 OR d1.rowNum in possibleIndex \n"
    ") \n"
    "SELECT d1.prevFpStart as fpStart, d1.prevBpEnd as bpEnd, d2.nextFpEnd as fpEnd, d2.nextBpStart as bpStart, \n"
    "d2.nextFpEnd - d1.prevFpStart as fpDuration, d1.prevBpEnd - d2.nextBpStart as bpDuration \n"
    "FROM possibleData d1 JOIN possibleData d2 ON d2.rowNum = d1.rowNum + 1 \n"
    "WHERE d1.prevFpStart >= ? AND d1.prevFpStart <= ?";

const std::string QUERY_HOST_METADATA_CANN_SQL =
    " select EAL.name, globalTid, type, max(depth) as maxDepth from CANN_API"
    " a join ENUM_API_TYPE EAL on a.type = EAL.id "
    " group by type, globalTid order by globalTid, type desc";
const std::string QUERY_HOST_METADATA_PYTORCH_SQL =
    " select 'pytorch' as name, globalTid, 'pytorch' as type,"
    " max(depth) as maxDepth from PYTORCH_API"
    " a group by globalTid order by globalTid";
const std::string QUERY_HOST_METADATA_OSRT_SQL =
    "SELECT 'OS Runtime API' AS name, globalTid, 'OS Runtime API' AS type, 0 AS maxDepth FROM OSRT_API"
    " a GROUP BY globalTid ORDER BY globalTid";
const std::string QUERY_HOST_METADATA_MSTX_SQL =
    "select 'MsTx' as name, globalTid,'MsTx' as type, max(depth) as maxDepth from MSTX_EVENTS a "
    " group by globalTid order by globalTid";
const std::string QUERY_HOST_METADATA_PYTHONGC_SQL =
    "select 'Python GC' as name, globalTid,'Python GC' as type,  0 as maxDepth from GC_RECORD a "
    " group by globalTid order by globalTid";

// 兼容老版本（1.0.0）
    const std::string QUERY_COMMUNICATION_GROUP_MAP_DB_1_0_SQL =
        "SELECT groupName, planeId, 'Plane ' || planeId as threadName FROM " + TABLE_COMMUNICATION_TASK_INFO + " "
        "GROUP BY groupName || planeId "
        "UNION "
        "SELECT op.groupName, -1 as planeId, 'Group ' || row_num || ' Communication' as threadName "
        "FROM " + TABLE_COMMUNICATION_OP + " op JOIN ( "
        "    SELECT groupName, row_number() OVER (ORDER BY groupName ASC) - 1 as row_num "
        "    FROM " + TABLE_COMMUNICATION_OP + " GROUP BY groupName "
        ") grp ON op.groupName = grp.groupName "
        "GROUP BY op.groupName";
    const std::string QUERY_COMMUNICATION_GROUP_MAP_DB_SQL =
        "    SELECT groupName, planeId, 'Plane ' || planeId as threadName FROM COMMUNICATION_TASK_INFO "
        "    GROUP BY groupName || planeId  "
        "    UNION "
        "    SELECT op.groupName, -1 as planeId, 'Group ' || strGroup.value || ' Communication' as threadName "
        "    FROM COMMUNICATION_OP op JOIN ( "
        "        SELECT groupName "
        "        FROM  COMMUNICATION_OP  GROUP BY groupName "
        "    ) grp ON op.groupName = grp.groupName "
        "    JOIN STRING_IDS strGroup ON op.groupName = strGroup.id "
        "    GROUP BY op.groupName ";

// 兼容老版本（1.0.0）
const std::string QUERY_COMMUNICATION_SUMMARY_DB_1_0_SQL =
    "WITH data AS ("
    "    SELECT *, row_number() OVER (ORDER BY groupName ASC, planeId ASC, start_time ASC) as row_num "
    "    FROM ("
    "        SELECT str1.value as name, task.startNs as start_time, task.endNs - task.startNs as duration, "
    "        task.endNs as end_time, groupName, planeId, 'Plane ' || planeId as thread_name, 0 as type "
    "        FROM " + TABLE_COMMUNICATION_TASK_INFO + " info "
    "        JOIN " + TABLE_STRING_IDS + " str1 ON info.taskType = str1.id "
    "        JOIN " + TABLE_TASK + " task ON info.globalTaskId = task.globalTaskId "
    "        UNION "
    "        SELECT str2.value as name, startNs as start_time, endNs - startNs as duration, "
    "        endNs as end_time, op.groupName, -1 as planeId, "
    "        'Group ' || row_num || ' Communication' as thread_name, 1 as type "
    "        FROM " + TABLE_COMMUNICATION_OP + " op "
    "        JOIN " + TABLE_STRING_IDS + " str2 ON op.opName = str2.id "
    "        JOIN ( "
    "            SELECT groupName, row_number() OVER (ORDER BY groupName ASC) - 1 as row_num "
    "            FROM " + TABLE_COMMUNICATION_OP + " GROUP BY groupName "
    "        ) grp ON op.groupName = grp.groupName "
    "    ) "
    ") ";
const std::string QUERY_COMMUNICATION_SUMMARY_DB_SQL =
    "  WITH data AS ("
    "      SELECT *, row_number() OVER (ORDER BY groupName ASC, planeId ASC, start_time ASC) as row_num "
    "  FROM ("
    "      SELECT str1.value as name, task.startNs as start_time, task.endNs - task.startNs as duration, "
    "      task.endNs as end_time, groupName, planeId, 'Plane ' || planeId as thread_name, 0 as type "
    "      FROM COMMUNICATION_TASK_INFO info "
    "      JOIN STRING_IDS str1 ON info.taskType = str1.id "
    "      JOIN TASK task ON info.globalTaskId = task.globalTaskId "
    "      UNION "
    "      SELECT str2.value as name, startNs as start_time, endNs - startNs as duration, "
    "      endNs as end_time, op.groupName, -1 as planeId,  "
    "      'Group ' || strGroup.value || ' Communication'  as thread_name,"
    "       1 as type "
    "      FROM COMMUNICATION_OP op "
    "      JOIN STRING_IDS str2 ON op.opName = str2.id "
    "      JOIN STRING_IDS strGroup ON op.groupName = strGroup.id "
    "      JOIN ( "
    "          SELECT groupName, row_number() OVER (ORDER BY groupName ASC) - 1 as row_num "
    "          FROM COMMUNICATION_OP GROUP BY groupName "
    "          )"
    "      grp ON op.groupName = grp.groupName "
    "      )"
    " ) ";
class TraceDatabaseSqlConst {
public:
    static std::string GenerateAclnnQueryDbSql(const Protocol::KernelDetailsParams &params)
    {
        std::string sql =
            "SELECT info.ROWID as id, s1.value as name, s2.value as op_type, task.taskType, "
            "task.startNs - ? as startTime, (task.endNs - task.startNs) / 1000 as duration, 'Ascend Hardware' as pid, "
            "task.streamId as tid, task.depth as depth "
            "FROM " + TABLE_COMPUTE_TASK_INFO + " info "
            "JOIN " + TABLE_TASK + " task ON info.globalTaskId = task.globalTaskId "
            "JOIN " + TABLE_STRING_IDS + " s1 ON info.name = s1.id "
            "JOIN " + TABLE_STRING_IDS + " s2 ON info.opType = s2.id "
            "WHERE s1.value IN ("
            "    SELECT str.value FROM " + TABLE_COMPUTE_TASK_INFO + " info "
            "    JOIN " + TABLE_STRING_IDS + " str ON info.name = str.id "
            "    WHERE str.value LIKE 'aclnn%' "
            "    GROUP BY str.value HAVING COUNT(str.value) >= ? "
            ") ORDER BY " + params.orderBy + " " + params.order;
        return sql;
    }

    static std::string GenerateOperatorDispatchQueryDbSql(const Protocol::KernelDetailsParams &params)
    {
        std::string sql =
            "SELECT"
            "  ca.ROWID as id, "
            "  s.value as name, "
            "  ca.startNs - ? as startTime, "
            "  (ca.endNs - ca.startNs) / 1000 as duration, "
            "  ca.globalTid as pid, "
            "  ca.type as tid, "
            "  ca.depth as depth "
            "FROM " + TABLE_CANN_API + " ca "
            "JOIN " + TABLE_ENUM_API_TYPE + " enum ON ca.type = enum.id "
            "JOIN " + TABLE_STRING_IDS + " s ON ca.name = s.id "
            "WHERE s.value LIKE '%aclopCompileAndExecute' "
            "ORDER BY " + params.orderBy + " " + params.order;
        return sql;
    }
};
}
#endif // PROFILER_SERVER_TRACEDATABASESQLCONST_H
