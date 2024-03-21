/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: defines declaration
 */

#ifndef PROFILER_SERVER_DBSQLDEFS_H
#define PROFILER_SERVER_DBSQLDEFS_H

#include <string>

namespace Dic {

// sql of timeline unit/counter
const static std::string HBM_UNIT_COUNTER_SQL = "select timestampNs-? as startTime,hbmId||'/'|| case when name='read' "
     " then 'Read' else 'Write' end as processName,  case when name='read' then '{\"Read(MB/s)\":' || bandwidth || '}' "
     " else '{\"Write(MB/s)\":' || bandwidth || '}' end as args from HBM main join ENUM_MEMORY on type = id "
     " where deviceId= ? and processName = ? AND startTime >= ? AND startTime <= ? ORDER BY timestampNs ASC";

const static std::string LLC_UNIT_COUNTER_SQL = "with main as (select timestampNs - ? as startTime, "
     " format('%s %s', llcId, case when name='read' then 'Read' else 'Write' end) as modeName,? as processName, "
     " throughput,hitRate from LLC join ENUM_MEMORY on mode = id where deviceId = ?"
     " AND startTime >= ? AND startTime <= ? and glob(modeName||'*', processName)) select startTime, "
     " case when glob('*Throughput', processName) then format('{\"Throughput(MB/s)\":%s}', throughput) "
     " else format('{\"Hit Rate(%%)\":%s}', hitRate) end as args from main ORDER BY startTime ASC";
const static std::string DDR_UNIT_COUNTER_SQL = "select timestampNs-? as startTime, case when ? = 'Read' "
     " then format('{\"Read(MB/s)\":%s}', read) else format('{\"Write(MB/s)\":%s}', write) end as args from DDR "
     " where deviceId = ? and startTime >= ? AND startTime <= ? ORDER BY startTime ASC";
const static std::string SOC_UNIT_COUNTER_SQL = "select timestampNs - ? as startTime, case when ? ='L2 Buffer Bw Level'"
     " then format('{\"L2 Buffer Bw Level\":%s}', l2BufferBwLevel) else "
     " format('{\"Mata Bw Level\":%s}', mataBwLevel) end as args from SOC_BANDWIDTH_LEVEL"
     " where deviceId = ? and startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
const static std::string PMU_UNIT_COUNTER_SQL = "with pn as (select ? as value) select timestampNs - ? as startTime, "
     " format('{\"value\":%s, \"acc_id\":%s}', case when pn.value='read_bandwidth' then readBandwidth "
     " when pn.value = 'write_bandwidth' then writeBandwidth when pn.value = 'read_ost' then readOst "
     " else writeOst end, accId) as args  from ACC_PMU join pn "
     " where deviceId = ? and startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
const static std::string NPU_UNIT_COUNTER_SQL = "with pn as (select ? as value) select timestampNs - ? as startTime, "
     " case type when 0 then 'APP*'  else 'Device*' end as typeName, format('{\"KB\":%s}',"
     " case when glob('*DDR', pn.value) then ddrUsage when glob('*HBM', pn.value) then "
     " hbmUsage else hbmUsage + ddrUsage end) as args from NPU_MEM join pn where deviceId = ? and "
     " glob(typeName, pn.value) and startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";

// sql of timeline unit/flowName
const static std::string HOST_FLOW_NAME_SQL = " with tmp as (select * from TASK where connectionId = ?) "
      " select tmp.globalTaskId as flowId, info.name, 's' as type from COMPUTE_TASK_INFO info "
      " join tmp on info.globalTaskId = tmp.globalTaskId "
      " UNION select tmp.globalTaskId as flowId, info.name, 's' as type from COMMUNICATION_TASK_INFO info "
      " join tmp on info.globalTaskId = tmp.globalTaskId\n"
      " UNION select info.opId as flowId, info.opName as name, 's' as type from COMMUNICATION_OP info "
      " where info.connectionId = ?;";

const static std::string HCCL_FLOW_NAME_SQL = "SELECT main.connectionId as flowId, api.name,'f' as type FROM TASK main "
      " join COMMUNICATION_TASK_INFO CTI on CTI.globalTaskId = main.globalTaskId "
      " join api on api.connectionId = main.connectionId "
      " WHERE main.globalTaskId = ? and CTI.planeId = ? UNION "
      " SELECT main.connectionId as flowId, api.name, 'f' as type  FROM COMMUNICATION_OP main  join API api "
      " on api.connectionId = main.connectionId  WHERE main.opId = ? and main.groupName||'group' = ?";

const static std::string HARDWARE_FLOW_NAME_SQL = "select c.name, c.connectionId as flowId, 'f' as type"
      " from " + TABLE_TASK + " main "
      " join " + TABLE_API + " c on c.connectionId = main.connectionId"
      " where main.globalTaskId = ?;";

// sql of timeline unit/flowDetail
const static std::string HOST_FLOW_DETAIL_SQL = " select name, type as tid, startNs as start, endNs - startNs as dur,"
       " connectionId as id, depth, globalTid as pid from API where connectionId = ?;";

const static std::string HCCL_FLOW_DETAIL_SQL = "select name, planeId as tid, startNs as start, endNs - startNs as dur,"
        " main.globalTaskId as id, depth, deviceId from TASK main join COMMUNICATION_TASK_INFO info "
        " on main.globalTaskId = info.globalTaskId where connectionId = ? and main.globalTaskId = ? "
        " UNION select opName as name, op.groupName||'group' as tid, op.startNs as start, op.endNs - op.startNs as dur,"
        " op.opId as id, 0 as depth, TASK.deviceId from COMMUNICATION_OP op join COMMUNICATION_TASK_INFO info "
        " on info.opId = op.opId join TASK on info.globalTaskId = TASK.globalTaskId\n"
        " where op.connectionId = ? and op.opId = ? group by op.opId;";

const static std::string HARDWARE_FLOW_DETAIL_SQL = "select name, streamId as tid, startNs as start,"
        " endNs - startNs as dur, main.globalTaskId as id, depth, deviceId from TASK main join COMPUTE_TASK_INFO info "
        " on main.globalTaskId = info.globalTaskId where connectionId = ? and main.globalTaskId = ?;";
};

#endif // PROFILER_SERVER_DBSQLDEFS_H
