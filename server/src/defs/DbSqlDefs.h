/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: defines declaration
 */

#ifndef PROFILER_SERVER_DBSQLDEFS_H
#define PROFILER_SERVER_DBSQLDEFS_H

#include <string>

namespace Dic {

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
const static std::string NIC_MEAT_DATA_SQL =
        "with main as (select funcId from # where deviceId = ? group by funcId) "
        "     select 'rx_bandwidth_effciency,rx_packets,rx_error_rate,rx_dropped_rate' as types, "
        " format('Port %s/rx', funcId) as name from main "
        "UNION select 'tx_bandwidth_effciency,tx_packets,tx_error_rate,tx_dropped_rate' as types,"
        " format('Port %s/tx', funcId) as name from main";


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
     " case type when 0 then 'APP*'  else 'Device*' end as typeName, format('{\"B\":%s}',"
     " case when glob('*DDR', pn.value) then ddrUsage when glob('*HBM', pn.value) then "
     " hbmUsage else hbmUsage + ddrUsage end) as args from NPU_MEM join pn where deviceId = ? and "
     " glob(typeName, pn.value) and startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
const static std::string SAMPLE_PMU_UNIT_COUNTER_SQL =
        "select timestampNs - ? as startTime, "
        " format('{\"freq(Mhz)\":%s, \"usage(%%)\":%s, \"totalCycle\":%s}', freq, usage, totalCycle) as args "
        " from SAMPLE_PMU_TIMELINE join STRING_IDS on coreType = id  where deviceId = ? and "
        "        format('%s Core %s', value, coreId) = ? and startTime >= ? AND startTime <= ? ORDER BY startTime;";
const static std::string NIC_UNIT_COUNTER_SQL =
        "with pn as (select ? as name) "
        "select timestampNs - ? as startTime, "
        "case when glob('*rx',pn.name) then json_object('rx_bandwidth_effciency',round(rxByteRate*8/bandwidth,4),"
        " 'rx_packets', rxPackets, 'rx_error_rate', round(rxErrors * 1.0 / rxPackets, 4), "
        " 'rx_dropped_rate', round(rxDropped * 1.0 / rxPackets, 4)) "
        "  else json_object('tx_bandwidth_effciency', round(txByteRate * 8 / bandwidth, 4), "
        "  'tx_packets', txPackets, 'tx_error_rate', round(txErrors * 1.0 / txPackets, 4), "
        "  'tx_dropped_rate', round(txDropped * 1.0 / txPackets, 4)) end as args "
        " from # join pn where deviceId = ? and glob('Port '||funcId||'/*', pn.name)"
        " and startTime >= ? AND startTime <= ? ORDER BY startTime;";

const static std::string HCCS_UNIT_COUNTER_SQL =
        "select timestampNs - ? as startTime, json_object('txThroughput(B/s)',round(txThroughput, 4),"
        " 'rxThroughput(B/s)',round(rxThroughput, 4)) as args "
        "from HCCS where deviceId = ? and startTime >= ? AND startTime <= ? ORDER BY startTime";

const static std::string PCIE_UNIT_COUNTER_SQL =
        "select timestampNs - ? as startTime, "
        " case ? when 'PCIe_post' then json_object('txAvg(B/s)',round(txPostAvg, 4),'rxAvg(B/s)',round(rxPostAvg, 4)) "
        "  when 'PCIe_nonpost' then json_object('txAvg(B/s)',round(txNonpostAvg,4),'rxAvg(B/s)',round(rxNonpostAvg,4)) "
        "   when 'PCIe_cpl' then json_object('txAvg(B/s)', round(txCplAvg, 4), 'rxAvg(B/s)', round(rxCplAvg, 4)) "
        "   else json_object('txAvg(B/s)', round(txNonpostLatencyAvg, 4), 'rxAvg(B/s)', round(rxNonpostLatencyAvg, 4)) "
        " end as args"
        " from PCIE where deviceId = ? and startTime >= ? AND startTime <= ? ORDER BY startTime;";

const static std::string AI_CORE_UNIT_COUNTER_SQL =
    "select timestampNs - ? as startTime, json_object('Mhz',round(freq, 4)) as args "
    "     from AICORE_FREQ where deviceId = ? and startTime >= ? AND startTime <= ? ORDER BY startTime;";

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
const static std::string TASK_UNIT_FLOW_SQL =
      " select task.ROWID as id, streamId as tid, depth, startNs - constValue.minTime as startTime, "
      "     endNs - startNs as duration, 'ASCEND HARDWARE' as pid, 'ASCEND HARDWARE' as metaType, name, "
      "     deviceId from TASK task join constValue join COMPUTE_TASK_INFO CTI "
      "     on task.globalTaskId = CTI.globalTaskId where task.connectionId = constValue.connectionId "
      "union all select op.ROWID as id,groupName||'group' as tid,0 as depth,op.startNs-constValue.minTime as startTime,"
      "     op.endNs - op.startNs as duration, 'HCCL' as pid, 'HCCL' as metaType, opName as name, "
      "     deviceId from COMMUNICATION_OP op join constValue join TASK task on task.connectionId = op.connectionId "
      "     where op.connectionId = constValue.connectionId group by opId "
      "order by startTime;";

// sql of timeline unit/flowName
const static std::string HOST_FLOW_NAME_SQL = " with tmp as (select * from TASK where connectionId = ?) "
      " select tmp.globalTaskId as flowId, info.name, 's' as type from COMPUTE_TASK_INFO info "
      " join tmp on info.globalTaskId = tmp.globalTaskId "
      " UNION select tmp.globalTaskId as flowId, info.name, 's' as type from COMMUNICATION_TASK_INFO info "
      " join tmp on info.globalTaskId = tmp.globalTaskId "
      " UNION select info.opId as flowId, info.opName as name, 's' as type from COMMUNICATION_OP info "
      " where info.ROWID = ?;";

const static std::string HCCL_FLOW_NAME_SQL = "SELECT main.connectionId as flowId, api.name,'f' as type FROM TASK main "
      " join COMMUNICATION_TASK_INFO CTI on CTI.globalTaskId = main.globalTaskId "
      " join api on api.connectionId = main.connectionId "
      " WHERE main.ROWID = ? and CTI.planeId = ? and main.startNs = ? UNION "
      " SELECT main.connectionId as flowId, api.name, 'f' as type  FROM COMMUNICATION_OP main join " + TABLE_CANN_API +
      " api on api.connectionId= main.connectionId WHERE main.ROWID = ? "
      " and main.groupName||'group' = ? and main.startNs = ?";

const static std::string HARDWARE_FLOW_NAME_SQL = "select c.name, c.connectionId as flowId, 'f' as type"
      " from " + TABLE_TASK + " main "
      " join " + TABLE_CANN_API + " c on c.connectionId = main.connectionId"
      " where main.ROWID = ? and main.startNs = ?;";

// sql of timeline unit/flowDetail
const static std::string HOST_FLOW_DETAIL_SQL = " select name, type as tid, startNs as start, endNs - startNs as dur,"
       " connectionId as id, depth, globalTid as pid from " + TABLE_CANN_API + " where ROWID = ?;";

const static std::string HCCL_FLOW_DETAIL_SQL = "select name, planeId as tid, startNs as start, endNs - startNs as dur,"
        " main.globalTaskId as id, depth, deviceId from TASK main join COMMUNICATION_TASK_INFO info "
        " on main.globalTaskId = info.globalTaskId where connectionId = ? and main.ROWID = ? "
        " UNION select opName as name, op.groupName||'group' as tid, op.startNs as start, op.endNs - op.startNs as dur,"
        " op.opId as id, 0 as depth, TASK.deviceId from COMMUNICATION_OP op join COMMUNICATION_TASK_INFO info "
        " on info.opId = op.opId join TASK on info.globalTaskId = TASK.globalTaskId "
        " where op.connectionId = ? and op.ROWID = ? group by op.opId;";

const static std::string HARDWARE_FLOW_DETAIL_SQL = "select name, streamId as tid, startNs as start,"
        " endNs - startNs as dur, main.globalTaskId as id, depth, deviceId from TASK main join COMPUTE_TASK_INFO info "
        " on main.globalTaskId = info.globalTaskId where connectionId = ? and main.ROWID = ?;";

const static std::string FULL_DB_UPDATE_TIME =
        "SELECT deviceId, startNs, endNs,'compute' AS type, CTI.ROWID AS id FROM TASK main JOIN "
        "main.COMPUTE_TASK_INFO CTI ON main.globalTaskId = CTI.globalTaskId UNION SELECT deviceId, "
        "opInfo.startNs, opInfo.endNs, 'communication' AS type, opInfo.ROWID AS id FROM COMMUNICATION_OP "
        "opInfo JOIN TASK ON TASK.connectionId = opInfo.connectionId ORDER BY startNs;";
};

#endif // PROFILER_SERVER_DBSQLDEFS_H
