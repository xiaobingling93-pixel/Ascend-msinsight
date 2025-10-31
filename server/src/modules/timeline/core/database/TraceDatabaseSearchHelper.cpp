/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "TraceDatabaseHelper.h"

std::string TraceDatabaseHelper::GetSearchSliceNameSql(bool isMatchExact, bool isMatchCase, std::string rankId,
                                                       const std::string& path)
{
    const std::string order = "ascend";
    const std::string orderByField = "timestamp";
    std::string sql;
    std::string nameMatch = " select id from STRING_IDS where ";
    std::string orderKey = orderByField == "timestamp" ? "startTime" : orderByField;
    std::string orderBy = " ORDER BY " + orderKey + (order == "ascend" ? " ASC" : "DESC");
    nameMatch.append(isMatchCase ? " value like " : "lower(value) like lower(");
    nameMatch.append(isMatchExact ? "?" : "'%'||?||'%'");
    nameMatch.append(isMatchCase ? " " : ")");
    std::string associationTaskSql;
    if (!TraceDatabaseHelper::IsDeviceIdUnique(path)) {
        associationTaskSql = "join tasks on op.connectionId = tasks.connectionId";
    }
    const std::string hostSql =
        " SELECT name, globalTid as pid,  'HOST' as metaType,  type as tid, startNs - minTime.value as startTime,endNs "
        "- startNs as duration, depth, api.id "
        " FROM (select globalTid, type, startNs, endNs, depth, cann.ROWID as id, name from " +
        TABLE_CANN_API +
        " cann join ids on ids.id = cann.name "
        " Union all select globalTid, domainId as type, startNs, endNs, depth, mstx.ROWID as id, message as name "
        " from " + TABLE_MSTX_EVENTS + " mstx join ids on ids.id = mstx.message "
        " UNION all select globalTid, 'pytorch' as type, startNs, endNs, depth, python.ROWID as id, name "
        " from " + TABLE_API + " python join ids on ids.id = python.name" +
        " UNION ALL SELECT globalTid, 'OSRT_API' AS type, startNs, endNs, 0 AS depth, osrt.ROWID AS id, name"
        " FROM " +
        TABLE_OSRT_API + " osrt JOIN ids ON ids.id = osrt.name) api join minTime ";
    std::string comSql = "select opName as name,'HCCL' as pid, 'HCCL' as metaType, groupName||'group' as tid,"
                         " startNs - minTime.value as startTime, endNs - startNs as duration, 0 as depth, op.ROWID"
                         " as id from COMMUNICATION_OP op join minTime " +
                         associationTaskSql + " join ids on ids.id = opName group by opId";
    sql = "with ids as (" + nameMatch +
          "), minTime as (select ? as value), "
          " tasks as (select ROWID, globalTaskId, taskType, 'Ascend Hardware' as pid, streamId as tid, connectionId, "
          " startNs - minTime.value as startTime, endNs - startNs as duration,depth from TASK join minTime "
          " where deviceId = ? ORDER BY startTime), "
          " com as (select opId, tasks.ROWID as id, 'HCCL' as pid, groupName || '_' || planeId as tid, "
          " startTime, duration, 0 as depth, info.taskType as name from COMMUNICATION_TASK_INFO info "
          " join tasks on info.globalTaskId=tasks.globalTaskId ORDER BY startTime) "
          " select * from ( select coalesce(compute.name, schedule.name, main.taskType) as name, main.pid, main.pid "
          " as metaType, main.tid, main.startTime, main.duration, main.depth, main.ROWID as id from tasks main "
          " left join COMPUTE_TASK_INFO compute on compute.globalTaskId = main.globalTaskId "
          " left join COMMUNICATION_SCHEDULE_TASK_INFO schedule ON main.globalTaskId = schedule.globalTaskId "
          " join ids on ids.id = coalesce(compute.name, schedule.name, main.taskType) "
          " union ALL select name, pid, pid as meatType, tid, startTime, duration, depth, com.id from com "
          " join ids on ids.id = com.name  union ALL " +
          comSql + " union ALL " + hostSql + ") allNames " + orderBy + " LIMIT 1 OFFSET ?";
    return sql;
}

std::string TraceDatabaseHelper::GetSearchAllSlicesDetailsSql(bool isMatchExact, bool isMatchCase,
                                                              const std::string& order, const std::string& orderByField,
                                                              const std::string& rankId)
{
    std::string sql;
    std::string nameMatch;
    std::string orderBy;
    std::string orderKey = orderByField == "timestamp" ? "startTime" : orderByField;
    if (order == "descend") {
        orderBy = " ORDER BY " + orderKey + " DESC";
    } else {
        orderBy = " ORDER BY " + orderKey + " ASC";
    }
    if (isMatchExact && isMatchCase) {
        nameMatch = "select id, value from STRING_IDS where value like ?";
    } else if (isMatchExact) {
        nameMatch = "select id, value from STRING_IDS where lower(value) like lower(?)";
    } else if (isMatchCase) {
        nameMatch = "select id, value from STRING_IDS where value like '%'||?||'%'";
    } else {
        nameMatch = "select id, value from STRING_IDS where lower(value) like lower('%'||?||'%')";
    }
    std::string communicationOpSql = GetComOpSliceDetailsSql(rankId);
    std::string mstxEventsSql = GetMsTxEventsSliceDetailSql();
    sql = "with ids as (" + nameMatch +
          "), minTime as (select ? as value),\n"
          " tasks as (select deviceId, TASK.ROWID, globalTaskId, taskType, 'Ascend Hardware' as pid, streamId as tid, "
          " startNs - minTime.value as startTime,endNs - startNs as duration,depth,connectionId from TASK join minTime "
          " where deviceId = ? ORDER BY startTime),\n"
          " com as (select deviceId, opId, tasks.ROWID as id, 'HCCL' as pid, groupName || '_' || planeId as tid,"
          " startTime, duration, 0 as depth, info.taskType as name"
          " from COMMUNICATION_TASK_INFO info join tasks on info.globalTaskId=tasks.globalTaskId "
          " ORDER BY startTime)\n"
          " select * from ( select deviceId, coalesce(compute.name, schedule.name, main.taskType) as name, main.pid,"
          " main.pid as metaType,"
          " main.tid, main.startTime, main.duration, main.depth, main.ROWID as id from tasks main\n"
          " left join COMPUTE_TASK_INFO compute on compute.globalTaskId = main.globalTaskId "
          " LEFT JOIN COMMUNICATION_SCHEDULE_TASK_INFO schedule ON main.globalTaskId = schedule.globalTaskId union ALL"
          " select deviceId,name, pid, pid as meatType, tid, startTime, duration, depth, id from com union ALL " +
          communicationOpSql +
          " UNION all select '' as deviceId, name, globalTid as pid, 'HOST' as metaType, type as tid, "
          "startNs - minTime.value AS startTime, endNs - startNs AS duration, depth, CANN_API.ROWID as id from "
          "CANN_API JOIN minTime UNION all " + mstxEventsSql +
          "UNION all select '' as deviceId, name, globalTid as pid,"
          "'HOST' as metaType, 'pytorch' as tid, "
          "startNs - minTime.value AS startTime, endNs - startNs AS duration, depth, PYTORCH_API.ROWID as id from "
          "PYTORCH_API JOIN minTime "
          "UNION ALL SELECT '' AS deviceId, name, globalTid AS pid, 'HOST' AS metaType, 'OSRT_API' AS tid, "
          "startNs - minTime.value AS startTime, endNs - startNs AS duration, 0 AS depth, osrt.ROWID AS id FROM " +
          TABLE_OSRT_API + " osrt JOIN minTime) allNames join ids on ids.id = allNames.name" + orderBy +
          " LIMIT ? OFFSET ?";
    return sql;
}

std::string TraceDatabaseHelper::GetSearchSliceNameCountSql(bool isMatchExact, bool isMatchCase, std::string rankId)
{
    std::string sql;
    std::string nameMatch;
    if (isMatchExact && isMatchCase) {
        nameMatch = "select id from STRING_IDS where value like ?";
    } else if (isMatchExact) {
        nameMatch = "select id from STRING_IDS where lower(value) like lower(?)";
    } else if (isMatchCase) {
        nameMatch = "select id from STRING_IDS where value like '%'||?||'%'";
    } else {
        nameMatch = "select id from STRING_IDS where lower(value) like lower('%'||?||'%')";
    }
    std::string hostSql = "select name from " + TABLE_CANN_API + " union all select message from  " +
                          TABLE_MSTX_EVENTS + " union all select name from  " + TABLE_API +
                          " UNION ALL SELECT name FROM " + TABLE_OSRT_API;

    std::string communicationOpSql;
    if (!TraceDatabaseHelper::IsDeviceIdUnique(rankId)) {
        communicationOpSql = "select opName as name from COMMUNICATION_OP op "
                             " join tasks on op.connectionId = tasks.connectionId group by opId";
    } else {
        communicationOpSql = "select opName as name from COMMUNICATION_OP op";
    }
    sql = "with ids as (" + nameMatch +
          "), "
          "     tasks as (select globalTaskId, taskType, connectionId from TASK where deviceId = ?), "
          "     com as (select opId, info.globalTaskId,info.taskType as name from COMMUNICATION_TASK_INFO info "
          " join tasks on  info.globalTaskId = tasks.globalTaskId), "
          "     compute as (select info.globalTaskId, name from COMPUTE_TASK_INFO info join tasks "
          " on  info.globalTaskId = tasks.globalTaskId), "
          " schedule as (select info.globalTaskId, name from COMMUNICATION_SCHEDULE_TASK_INFO info left join tasks "
          " on info.globalTaskId = tasks.globalTaskId)"
          "select count(1) from ( "
          "    select coalesce(compute.name, schedule.name, main.taskType) as name from tasks main "
          "         left join compute on compute.globalTaskId = main.globalTaskId "
          " left join schedule ON main.globalTaskId = schedule.globalTaskId"
          "    union ALL select name from com "
          "    union ALL " +
          communicationOpSql + " union ALL " + hostSql + ") allNames join ids on id = allNames.name;";
    return sql;
}

std::string TraceDatabaseHelper::GetSearchCountWithLockSql(const SearchCountParams& params,
                                                           const std::vector<TrackQuery>& trackQuery)
{
    std::string sql;
    std::string nameMatch;
    if (params.isMatchExact && params.isMatchCase) {
        nameMatch = "select id from STRING_IDS where value like ?";
    } else if (params.isMatchExact) {
        nameMatch = "select id from STRING_IDS where lower(value) like lower(?)";
    } else if (params.isMatchCase) {
        nameMatch = "select id from STRING_IDS where value like '%'||?||'%'";
    } else {
        nameMatch = "select id from STRING_IDS where lower(value) like lower('%'||?||'%')";
    }
    sql = "with ids as (" + nameMatch + ") ";
    std::vector<std::string> sqls;
    for (const auto& item : trackQuery) {
        std::string tempSql = GetSingleSearchCountLockRangeSql(params, item);
        if (!tempSql.empty()) {
            sqls.emplace_back(tempSql);
        }
    }
    sql = sql + StringUtil::join(sqls, " UNION ALL ");
    return sql;
}

std::string TraceDatabaseHelper::GetSingleSearchCountLockRangeSql(const SearchCountParams& params,
                                                                  const TrackQuery& item)
{
    PROCESS_TYPE type = STR_TO_ENUM<PROCESS_TYPE>(item.metaType).value();
    std::string tempSql;
    if (type == PROCESS_TYPE::API) {
        tempSql = "SELECT count(1) FROM (SELECT name from " + TABLE_API +
                  " WHERE globalTid = ? AND startNs >= ? AND endNs <= ?) api join ids on id = api.name ";
    } else if (type == PROCESS_TYPE::CANN_API) {
        tempSql = "SELECT count(1) FROM (SELECT name from " + TABLE_CANN_API +
                  " WHERE globalTid = ? AND type = ? AND startNs >= ? AND endNs <= ?) cann join ids on id = cann.name ";
    } else if (type == PROCESS_TYPE::MS_TX) {
        tempSql = "SELECT count(1) FROM (SELECT message from " + TABLE_MSTX_EVENTS +
                  " WHERE globalTid = ? AND startNs >= ? AND endNs <= ?) mstx join ids on id = mstx.message ";
    } else if (type == PROCESS_TYPE::OSRT_API) {
        tempSql = "SELECT count(1) FROM (SELECT name from " + TABLE_OSRT_API +
                  " WHERE globalTid = ? AND startNs >= ? AND endNs <= ?) osrt join ids on id = osrt.name ";
    } else if (type == PROCESS_TYPE::ASCEND_HARDWARE) {
        tempSql = "SELECT count(1) FROM (SELECT coalesce(c.name, m.message, s.name, main.taskType) as "
                  "name FROM " + TABLE_TASK +
                  " main "
                  " left join " + TABLE_COMPUTE_TASK_INFO +
                  " c on c.globalTaskId = main.globalTaskId "
                  " left join " + TABLE_MSTX_EVENTS +
                  " m on "
                  " (m.connectionId = main.connectionId and  m.connectionId != " +
                  WRONG_DATA + " ) left join " + TABLE_COMMUNICATION_SCHEDULE_TASK +
                  " s on main.globalTaskId = s.globalTaskId WHERE main.deviceId = ? AND main.streamId = ? AND "
                  "main.startNs >= ? AND main.endNs <= ?) hadware  join ids on id = hadware.name ";
    } else if (type == PROCESS_TYPE::HCCL) {
        if (StringUtil::EndWith(item.threadId, "group")) {
            tempSql = "SELECT count(1) FROM (SELECT opName as name from " + TABLE_COMMUNICATION_OP +
                      " WHERE groupName = ? AND startNs >= ? AND endNs <= ?) op join ids on id = op.name ";
        } else {
            tempSql = "SELECT count(1) FROM (SELECT ci.taskType as name from TASK main left join " +
                      TABLE_COMMUNICATION_TASK_INFO + " ci on ci.globalTaskId = main.globalTaskId " +
                      " WHERE main.deviceId = ? and ci.groupName = ? AND ci.planeId = ? AND main.startNs >= ? AND "
                      "main.endNs <= ?) info join ids on id = info.name ";
        }
    }
    return tempSql;
}
