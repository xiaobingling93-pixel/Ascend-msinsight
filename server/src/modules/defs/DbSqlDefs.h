/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: defines declaration
 */

#ifndef PROFILER_SERVER_DBSQLDEFS_H
#define PROFILER_SERVER_DBSQLDEFS_H
#include <string>
#include "ConstantDefs.h"

namespace Dic {
    // init full db
const static std::map<std::string, std::string> FULL_DB_TABLE_MAP = {
    {TABLE_TASK, "create TEMPORARY table if not exists TASK( startNs INTEGER, endNs INTEGER, deviceId INTEGER, "
                 " connectionId INTEGER, globalTaskId INTEGER, globalPid INTEGER, taskType INTEGER,contextId INTEGER, "
                 " streamId INTEGER, taskId INTEGER, modelId INTEGER, depth integer);"},
    {TABLE_CANN_API, "create TEMPORARY table if not exists CANN_API(startNs INTEGER, endNs INTEGER, type INTEGER, "
                     " globalTid INTEGER, connectionId INTEGER primary key, name INTEGER, depth integer);  "},
    {TABLE_COMMUNICATION_OP, "create TEMPORARY table if not exists COMMUNICATION_OP( opName INTEGER, startNs INTEGER, "
                 " endNs INTEGER, connectionId INTEGER, groupName INTEGER, opId INTEGER primary key, "
                 " relay INTEGER, retry INTEGER, dataType INTEGER, algType INTEGER, count NUMERIC, waitNs INTEGER);  "},
    {TABLE_COMMUNICATION_TASK_INFO, "create TEMPORARY table if not exists COMMUNICATION_TASK_INFO( name INTEGER, "
                 " globalTaskId INTEGER, taskType INTEGER, planeId INTEGER, groupName INTEGER, notifyId INTEGER,"
                 " rdmaType INTEGER, srcRank INTEGER, dstRank INTEGER, transportType INTEGER, size INTEGER, "
                 " dataType INTEGER, linkType INTEGER, opId INTEGER);  "},
    {TABLE_COMPUTE_TASK_INFO, "create TEMPORARY table if not exists COMPUTE_TASK_INFO( name INTEGER, "
                 " globalTaskId INTEGER primary key, blockDim INTEGER, mixBlockDim INTEGER, taskType INTEGER, "
                 " opType INTEGER, inputFormats INTEGER, inputDataTypes INTEGER, inputShapes INTEGER, "
                 " outputFormats INTEGER, outputDataTypes INTEGER, outputShapes INTEGER, attrInfo INTEGER, "
                 " waitNs INTEGER);  "},
    {TABLE_CONNECTION_IDS, "create TEMPORARY table if not exists CONNECTION_IDS( id INTEGER, connectionId INTEGER);  "},
    {TABLE_ENUM_API_TYPE, "create TEMPORARY table if not exists ENUM_API_TYPE( id INTEGER primary key, name TEXT);  "},
    {TABLE_API, "create TEMPORARY table if not exists PYTORCH_API(startNs TEXT, endNs TEXT,globalTid INTEGER, "
                " connectionId INTEGER, name INTEGER, sequenceNumber INTEGER, fwdThreadId INTEGER,inputDtypes INTEGER, "
                " inputShapes INTEGER, callchainId INTEGER, depth integer);  "},
    {TABLE_MSTX_EVENTS, "create TEMPORARY table if not exists MSTX_EVENTS(startNs INTEGER,endNs INTEGER, "
                        " eventType INTEGER,rangeId INTEGER, category INTEGER, message INTEGER, globalTid INTEGER, "
                        " endGlobalTid INTEGER, domainId INTEGER, connectionId INTEGER, depth integer); "},
    {TABLE_PYTORCH_CALLCHAINS,"create TEMPORARY table if not exists PYTORCH_CALLCHAINS(id INTEGER, stack INTEGER"
                              " , stackDepth INTEGER );"},
    {TABLE_COMMUNICATION_SCHEDULE_TASK, "create TEMPORARY table if not exists COMMUNICATION_SCHEDULE_TASK_INFO("
                                        "name INTEGER, globalTaskId INTEGER primary key, taskType INTEGER, "
                                        "opType INTEGER);"},
    {TABLE_NPU_INFO, "create TEMPORARY table if not exists NPU_INFO(id INTEGER primary key, name TEXT);"}
};

    // sql of metadata counter
const static std::string HBM_MEAT_DATA_SQL =
        "select case when value='read' then 'Read(B/s)' else 'Write(B/s)' end as types, "
        "    hbmId || '/' || case when value='read' then 'Read' else 'Write' end as name "
        "FROM # join STRING_IDS on type = id WHERE deviceId = ? GROUP BY hbmId, type";
const static std::string LLC_MEAT_DATA_SQL =
        "with main as (select llcId, mode from # where deviceId = ? group by llcId, mode)"
        " select 'Throughput(B/s)' as types, llcId || ' ' || case when value='read' then 'Read' else"
        " 'Write' end || '/Throughput' as name  from main join STRING_IDS on mode = id "
        " UNION select 'Hit Rate(%)' as types, llcId || ' ' || case when value='read' then 'Read' else"
        " 'Write' end || '/Hit Rate' as name  from main join STRING_IDS on mode = id ";
const static std::string SAMPLE_PMU_MEAT_DATA_SQL =
        "with main as (select coreId, coreType from # "
        " where deviceId = ? group by coreType, coreId) "
        "  select 'freq(Mhz),usage(%),totalCycle' as types, format('%s Core %s', value, coreId) as name  "
        " from main join STRING_IDS on coreType = id";


    // sql of timeline unit/counter
const static std::string HBM_UNIT_COUNTER_SQL = "select timestampNs-? as startTime,hbmId||'/'|| case when value='read' "
     " then 'Read' else 'Write' end as processName, case when value='read' then '{\"Read(B/s)\":' || bandwidth || '}' "
     " else '{\"Write(B/s)\":' || bandwidth || '}' end as args from HBM main join STRING_IDS on type = id "
     " where deviceId= ? and processName = ? AND startTime >= ? AND startTime <= ? ORDER BY timestampNs ASC";

const static std::string LLC_UNIT_COUNTER_SQL = "with main as (select timestampNs - ? as startTime, "
     " format('%s %s', llcId, case when value='read' then 'Read' else 'Write' end) as modeName,? as processName, "
     " throughput,hitRate from LLC join STRING_IDS on mode = id where deviceId = ?"
     " AND startTime >= ? AND startTime <= ? and glob(modeName||'*', processName)) select startTime, "
     " case when glob('*Throughput', processName) then format('{\"Throughput(B/s)\":%s}', throughput) "
     " else format('{\"Hit Rate(%%)\":%s}', hitRate) end as args from main ORDER BY startTime ASC";
const static std::string DDR_UNIT_COUNTER_SQL = "select timestampNs-? as startTime, case when ? = 'Read' "
     " then format('{\"Read(B/s)\":%s}', read) else format('{\"Write(B/s)\":%s}', write) end as args from DDR "
     " where deviceId = ? and startTime >= ? AND startTime <= ? ORDER BY startTime ASC";
const static std::string SOC_UNIT_COUNTER_SQL = "select timestampNs - ? as startTime, case when ? ='L2 Buffer Bw Level'"
     " then format('{\"L2 Buffer Bw Level\":%s}', l2BufferBwLevel) else "
     " format('{\"Mata Bw Level\":%s}', mataBwLevel) end as args from SOC_BANDWIDTH_LEVEL"
     " where deviceId = ? and startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
const static std::string ACC_PMU_UNIT_COUNTER_SQL =
     "with pn as (select ? as value) select timestampNs - ? as startTime, "
     " format('{\"value\":%s, \"acc_id\":%s}', case when pn.value='readBwLevel' then readBwLevel "
     " when pn.value = 'writeBwLevel' then writeBwLevel when pn.value = 'readOstLevel' then readOstLevel "
     " else writeOstLevel end, accId) as args  from ACC_PMU join pn "
     " where deviceId = ? and startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
const static std::string NPU_UNIT_COUNTER_SQL = "with pn as (select ? as value) select timestampNs - ? as startTime, "
     " case s.value when 'app' then 'APP*'  else 'Device*' end as typeName, format('{\"B\":%s}',"
     " case when glob('*DDR', pn.value) then ddr when glob('*HBM', pn.value) then "
     " hbm else hbm + ddr end) as args from NPU_MEM join STRING_IDS s on type = s.id join pn where deviceId = ? and "
     " glob(typeName, pn.value) and startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
const static std::string SAMPLE_PMU_UNIT_COUNTER_SQL =
        "select timestampNs - ? as startTime, "
        " format('{\"freq(Mhz)\":%s, \"usage(%%)\":%s, \"totalCycle\":%s}', freq, usage, totalCycle) as args "
        " from SAMPLE_PMU_TIMELINE join STRING_IDS on coreType = id  where deviceId = ? and "
        "        format('%s Core %s', value, coreId) = ? and startTime >= ? AND startTime <= ? ORDER BY startTime;";

const static std::string AI_CORE_UNIT_COUNTER_SQL =
    "select timestampNs - ? as startTime, json_object('Mhz',round(freq, 4)) as args "
    "     from AICORE_FREQ where deviceId = ? and startTime >= ? AND startTime <= ? ORDER BY startTime;";

// sql of same operate detail
const static std::string ASCEND_SAME_NAME_DETAIL_SQL =
    "with nameIds as (select id, value as realName from STRING_IDS where value = ?) "
    "select startNs - ? as timestamp, endNs - startNs as duration, depth, main.ROWID as id , streamId as tid "
    "from TASK  main "
    "     left join COMPUTE_TASK_INFO c on c.globalTaskId = main.globalTaskId "
    "     join nameIds on coalesce(c.name, main.taskType) = id  where deviceId = ? and streamId in (";
const static std::string HCCL_SAME_NAME_DETAIL_SQL_PART1 =
    "with nameIds as (select id, ? as minTime, ? as rankId, ? as startTime, ? as endTime "
    "                   from STRING_IDS where value = ?) "
    "select startNs-minTime as timestamp,endNs-startNs as duration,0 as depth, main.ROWID as id , "
    " groupName || '_' || planeId as tid "
    "from  TASK main "
    "   join COMMUNICATION_TASK_INFO c on c.globalTaskId = main.globalTaskId join nameIds on c.taskType = id "
    " where deviceId=rankId and groupName || '_' || planeId in (";
const static std::string COM_OP_SAME_NAME_DETAIL_SQL_NOT_UNIQUE_DEVICE =
    ") and timestamp+duration >= startTime AND timestamp <= endTime "
    " UNION ALL select op.startNs - minTime as timestamp, op.endNs - op.startNs as duration, 0 as depth, "
    " op.ROWID as id , c.groupName || '_' || c.planeId as tid from COMMUNICATION_OP op "
    " join TASK main on op.connectionId = main.connectionId "
    " join COMMUNICATION_TASK_INFO c on c.globalTaskId = main.globalTaskId "
    " join nameIds on op.opName = id where deviceId = rankId and op.groupName||'group' in (";
const static std::string COM_OP_SAME_NAME_DETAIL_SQL_UNIQUE_DEVICE =
    ") and timestamp+duration >= startTime AND timestamp <= endTime "
    " UNION ALL select op.startNs - minTime as timestamp, op.endNs - op.startNs as duration, 0 as depth, "
    " op.ROWID as id , null as tid from COMMUNICATION_OP op "
    " join nameIds on op.opName = id where op.groupName||'group' in (";
// sql of singleUnitFlow
const static std::string PYTORCH_UNIT_FLOW_SQL =
      " select api.ROWID as id, 'pytorch' as tid, depth, startNs - constValue.minTime as startTime, "
      "     endNs - startNs as duration, globalTid as pid, 'PYTORCH_API' as metaType, name, "
      "     '' as deviceId from PYTORCH_API api join constValue "
      "     join CONNECTION_IDS ids on api.connectionId = ids.id and ids.connectionId = constValue.connectionId ";
const static std::string CANN_UNIT_FLOW_SQL =
      " select api.ROWID as id, type as tid, depth, startNs - constValue.minTime as startTime, "
      "     endNs - startNs as duration, globalTid as pid, 'CANN_API' as metaType, name, "
      "     '' as deviceId from CANN_API api join constValue "
      "     where api.connectionId = constValue.connectionId ";
const static std::string MSTX_UNIT_FLOW_SQL =
        " select api.ROWID as id, 'MsTx' as tid, depth, startNs - constValue.minTime as startTime, "
        "     endNs - startNs as duration, globalTid as pid, 'MSTX_EVENTS' as metaType, message as name, "
        "     '' as deviceId from MSTX_EVENTS api join constValue "
        "     where api.connectionId = constValue.connectionId and api.connectionId != 4294967295";
const static std::string TASK_UNIT_FLOW_SQL =
      " select task.ROWID as id, streamId as tid, depth, startNs - constValue.minTime as startTime, "
      "     endNs - startNs as duration, 'Ascend Hardware' as pid, 'Ascend Hardware' as metaType, name, "
      "     deviceId from TASK task join constValue join COMPUTE_TASK_INFO CTI "
      "     on task.globalTaskId = CTI.globalTaskId where task.connectionId = constValue.connectionId "
      " and task.connectionId != " + WRONG_DATA + " "
      " union all select task.ROWID as id, streamId as tid, task.depth,task.startNs - constValue.minTime as startTime, "
      " task.endNs - task.startNs as duration, 'Ascend Hardware' as pid, 'Ascend Hardware' as metaType, taskType, "
      "         deviceId from TASK task join constValue join MSTX_EVENTS CTI "
      "         on task.connectionId = CTI.connectionId where task.connectionId = constValue.connectionId  "
      " and task.connectionId != " + WRONG_DATA;
const static std::string COM_OP_UNIT_FLOW_SQL =
      " select op.ROWID as id,groupName||'group' as tid,0 as depth,op.startNs-constValue.minTime as startTime,"
      "     op.endNs - op.startNs as duration, 'HCCL' as pid, 'HCCL' as metaType, opName as name, "
      "     deviceId from COMMUNICATION_OP op join constValue join TASK task on task.connectionId = op.connectionId "
      "     where op.connectionId = constValue.connectionId group by opId ";
const static std::string COM_OP_UNIT_FLOW_SQL_UNIQUE_DEVICE =
      " select op.ROWID as id,groupName||'group' as tid,0 as depth,op.startNs-constValue.minTime as startTime,"
      "     op.endNs - op.startNs as duration, 'HCCL' as pid, 'HCCL' as metaType, opName as name, "
      "     ? as deviceId from COMMUNICATION_OP op join constValue "
      "     where op.connectionId = constValue.connectionId group by opId ";

// full_db_update_wait_time
const static std::string FULL_DB_UPDATE_TIME =
        "SELECT deviceId, startNs, endNs,'compute' AS type, CTI.ROWID AS id FROM TASK main JOIN "
        " COMPUTE_TASK_INFO CTI ON main.globalTaskId = CTI.globalTaskId UNION SELECT deviceId, "
        "opInfo.startNs, opInfo.endNs, 'communication' AS type, opInfo.ROWID AS id FROM COMMUNICATION_OP "
        "opInfo JOIN TASK ON TASK.connectionId = opInfo.connectionId ORDER BY startNs;";

// QueryThreadsByPid
const static std::string ASCEND_THREADS_BY_PID =
            "select main.startNs,main.endNs - main.startNs as duration,main.endNs, "
            " coalesce(c.name, m.message, s.name, main.taskType) as name, main.depth "
            " from " + TABLE_TASK + " main left join " + TABLE_COMPUTE_TASK_INFO +
            " c on c.globalTaskId = main.globalTaskId "
            " left join " + TABLE_MSTX_EVENTS + " m on "
            " (m.connectionId = main.connectionId and main.connectionId != " + WRONG_DATA + " ) " +
            " left join " + TABLE_COMMUNICATION_SCHEDULE_TASK + " s on main.globalTaskId = s.globalTaskId"
            " where deviceId = ? and streamId = ? and main.endNs >= ? AND main.startNs <= ?"
            " ORDER BY main.depth ASC, main.startNs ASC;";

const static std::string HCCL_THREADS_BY_PID =
            "with sub as ("
            "select startNs, endNs-startNs as duration, endNs, info.taskType as name from " + TABLE_TASK +
            " main join " + TABLE_COMMUNICATION_TASK_INFO +
            " info on info.globalTaskId = main.globalTaskId\n"
            " where deviceId = ? and groupName || '_' || planeId = ?\n"
            " UNION select startNs,endNs-startNs as duration,endNs,opInfo.opName"
            " from COMMUNICATION_OP opInfo"
            " where groupName||'group' = ?) select * from sub where sub.endNs >= ? "
            " and sub.startNs <= ?;";

const static std::string CANN_API_THREADS_BY_PID =
              "select startNs, endNs - startNs as duration, endNs, name, depth from CANN_API"
              " main where type = ? and globalTid = ? and endNs >= ? AND startNs <= ?"
              " ORDER BY depth ASC, startNs ASC;";

const static std::string API_THREADS_BY_PID =
        "select startNs, endNs - startNs as duration, endNs, name, depth from PYTORCH_API "
        " main where globalTid = ? and endNs >= ? AND startNs <= ?"
        " ORDER BY depth ASC, startNs ASC;";

const static std::string OVERLAP_ANALYSIS_THREAD_BY_PID =
        "select startNs, endNs - startNs as duration, endNs, 'OVERLAP_ANALYSIS'||type as name, "
        " 0 as depth from " + TABLE_OVERLAP_ANALYSIS + " where deviceId = ? and type = ? "
        " and endNs >= ? AND startNs <= ? ORDER BY startNs;";

const static std::string MS_TX_THREAD_BY_PID =
        "select startNs, endNs - startNs as duration,endNs,message as name,depth from " +
        TABLE_MSTX_EVENTS +
        " where globalTid = ? and endNs >= ? AND startNs <= ? ORDER BY startNs";

class DbSqlDefs {
public:
static std::string GetConnectionCatSql()
{
    std::string sql =
               "CREATE TABLE IF NOT EXISTS connectionCats as "
               " with operateConnIds as (select op.connectionId from COMMUNICATION_OP op "
               "   UNION all select connectionId from TASK task "
               " join COMPUTE_TASK_INFO CTI on task.globalTaskId = CTI.globalTaskId) ";
    sql.append(" select api.connectionId, 'HostToDevice' as cat  from CANN_API api " // cann侧
               " join operateConnIds on api.connectionId = operateConnIds.connectionId ");
    sql.append(" union select api.connectionId, 'async_npu' as cat from CONNECTION_IDS api " // pytorch侧
               " join operateConnIds on api.connectionId = operateConnIds.connectionId "
               " union select conn.connectionId, case ids.value when 'Enqueue' then 'async_task_queue'"
               " else 'fwdbwd' end as cat from CONNECTION_IDS conn join PYTORCH_API PA "
               " on conn.id = PA.connectionId join STRING_IDS ids on ids.id = PA.name "
               " group by conn.connectionId having count(1) > 1 ");
    sql.append(" union select api.connectionId, 'MsTx' as cat  from MSTX_EVENTS api " // 打点侧
               "      join Task task on api.connectionId = task.connectionId where api.connectionId != 4294967295 ");
    return sql;
}

static std::string GetQueryCannApiLocationSql()
{
    std::string sql = "with constValue as (select ? as minTime), "
                      "     rankIds as (select deviceId, globalPid from TASK group by globalPid) ";
    sql.append(" select connectionCats.cat, connectionCats.connectionId, api.ROWID as id, api.type as tid,"
        " depth, startNs - constValue.minTime as startTime, endNs - startNs as duration, globalTid as pid, "
        " 'CANN_API' as metaType, name, deviceId from " + TABLE_CANN_API + " api join constValue "
        " join connectionCats on api.connectionId = connectionCats.connectionId "
        " join rankIds on api.globalTid >> 32 = rankIds.globalPid "
        "  where cat = 'HostToDevice'");
    sql.append(" order by startTime");
    return sql;
}

static std::string GetQueryPyApiLocationSql()
{
    std::string sql = "with constValue as (select ? as minTime), "
                      "     rankIds as (select deviceId, globalPid from TASK group by globalPid) ";
    sql.append(" select connectionCats.cat, connectionCats.connectionId, api.ROWID as id, 'pytorch' as tid,depth,"
               " startNs - constValue.minTime as startTime, endNs - startNs as duration, globalTid as pid,"
               " 'PYTORCH_API' as metaType, name, deviceId from PYTORCH_API api join constValue "
               " join CONNECTION_IDS ids on api.connectionId = ids.id "
               " join connectionCats on ids.connectionId = connectionCats.connectionId "
               " join rankIds on api.globalTid >> 32 = rankIds.globalPid "
               " where cat = 'async_task_queue' or cat = 'fwdbwd' or cat = 'async_npu' ");
    sql.append(" order by startTime");
    return sql;
}

static std::string GetQueryMstxApiLocationSql()
{
    std::string sql = "with constValue as (select ? as minTime), "
                      "     rankIds as (select deviceId, globalPid from TASK group by globalPid) ";
    sql.append(" select connectionCats.cat, connectionCats.connectionId, api.ROWID as id, 'MsTx' as tid, "
               "  depth, startNs - constValue.minTime as startTime, endNs - startNs as duration, globalTid as pid, "
               "       'MSTX_EVENTS' as metaType, message as name, deviceId from MSTX_EVENTS api join constValue "
               "        join connectionCats on api.connectionId = connectionCats.connectionId "
               " join rankIds on api.globalTid >> 32 = rankIds.globalPid "
               " where cat = 'MsTx'");
    sql.append(" order by startTime");
    return sql;
}
};
};

#endif // PROFILER_SERVER_DBSQLDEFS_H
