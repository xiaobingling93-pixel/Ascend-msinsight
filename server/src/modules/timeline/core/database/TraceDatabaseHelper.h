/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACEDATABASEHELPER_H
#define PROFILER_SERVER_TRACEDATABASEHELPER_H

#include "TableDefs.h"
#include "SqlitePreparedStatement.h"
#include "VirtualTraceDatabase.h"
#include "NumberUtil.h"
#include "ProtocolEnumUtil.h"
#include "CommonDefs.h"
#include "DbSqlDefs.h"
#include "JsonUtil.h"
#include "ServerLog.h"
#include "DataBaseManager.h"

namespace Dic::Module::Timeline {
using namespace Protocol;

class TraceDatabaseHelper {
public:
    static std::unique_ptr<SqliteResultSet> QueryThreadsByPid(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                              const Protocol::UnitThreadsParams &requestParams,
                                                              uint64_t minTimestamp)
    {
        std::string sql;
        auto processType = GetProcessType(requestParams.metaType);
        Protocol::ExtremumTimestamp extremumTimestamp {};
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                QueryExtremumTimeOfFirstDepth(stmt, requestParams, minTimestamp, extremumTimestamp);
                sql = "select startNs,endNs - startNs as duration,endNs,coalesce(c.name, main.taskType) as name, depth "
                      " from " + TABLE_TASK + " main "
                      " left join " + TABLE_COMPUTE_TASK_INFO +
                    " c on c.globalTaskId = main.globalTaskId"
                    " where deviceId = ? and streamId = ? and endNs >= ? AND startNs <= ?"
                    " ORDER BY depth ASC, startNs ASC;";
                return ExecuteQuery(stmt, sql, requestParams.rankId, requestParams.tid,
                                    extremumTimestamp.minTimestamp, extremumTimestamp.maxTimestamp);
            case PROCESS_TYPE::HCCL:
                sql = "with sub as ("
                      "select startNs, endNs-startNs as duration, endNs, info.name as name from " + TABLE_TASK + " main"
                      " join "+ TABLE_COMMUNICATION_TASK_INFO + " info on info.globalTaskId = main.globalTaskId\n"
                      " where deviceId = ? and planeId = ?\n"
                      " UNION select startNs,endNs-startNs as duration,endNs,opInfo.opName from COMMUNICATION_OP opInfo"
                      " where groupName||'group' = ?) select * from sub where sub.endNs >= ? "
                      " and sub.startNs <= ?;";
                return ExecuteQuery(stmt, sql, requestParams.rankId, requestParams.tid, requestParams.tid,
                                    requestParams.startTime + minTimestamp, requestParams.endTime + minTimestamp);
            case PROCESS_TYPE::CANN_API:
                sql = "select startNs, endNs - startNs as duration, endNs, name, depth from " + requestParams.metaType +
                      " main where type = ? and globalTid = ? and endNs >= ? AND startNs <= ?"
                      " ORDER BY depth ASC, startNs ASC;";
                return ExecuteQuery(stmt, sql, requestParams.tid, requestParams.pid,
                                    requestParams.startTime + minTimestamp, requestParams.endTime + minTimestamp);
            case PROCESS_TYPE::API:
                sql = "select startNs, endNs - startNs as duration, endNs, name, depth from " + requestParams.metaType +
                      " main where globalTid = ? and endNs >= ? AND startNs <= ?"
                      " ORDER BY depth ASC, startNs ASC;";
                return ExecuteQuery(stmt, sql, requestParams.pid,
                                    requestParams.startTime + minTimestamp, requestParams.endTime + minTimestamp);
            default:
                throw DatabaseException("unsupported type!");
        }
    };

    static std::unique_ptr<SqliteResultSet> QueryFlowName(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                          const Protocol::UnitFlowNameParams &requestParams,
                                                          const uint64_t minTimestamp)
    {
        std::string sql;
        auto processType = GetProcessType(requestParams.metaType);
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                return ExecuteQuery(stmt, HARDWARE_FLOW_NAME_SQL, requestParams.id,
                                    requestParams.startTime + minTimestamp);
            case PROCESS_TYPE::HCCL:
                stmt->BindParams(requestParams.id, requestParams.tid, requestParams.startTime + minTimestamp);
                return ExecuteQuery(stmt, HCCL_FLOW_NAME_SQL, requestParams.id, requestParams.tid,
                                    requestParams.startTime + minTimestamp);
            case PROCESS_TYPE::CANN_API:
            case PROCESS_TYPE::API:
                return ExecuteQuery(stmt, HOST_FLOW_NAME_SQL, requestParams.id, requestParams.id);
            default:
                throw DatabaseException("unsupported type!");
        }
    };

    static bool QueryFlowDetail(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                Protocol::FlowLocation &flowBody, PROCESS_TYPE processType,
                                const Protocol::UnitFlowParams &requestParams,
                                std::map<std::string, std::string> &stringCache)
    {
        std::string sql;
        std::unique_ptr<SqliteResultSet> resultSet;
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                resultSet = ExecuteQuery(stmt, HARDWARE_FLOW_DETAIL_SQL, requestParams.flowId, requestParams.id);
                break;
            case PROCESS_TYPE::HCCL:
                resultSet = ExecuteQuery(stmt, HCCL_FLOW_DETAIL_SQL, requestParams.flowId, requestParams.id,
                                         requestParams.flowId, requestParams.id);
                break;
            case PROCESS_TYPE::CANN_API:
            case PROCESS_TYPE::API:
                resultSet = ExecuteQuery(stmt, HOST_FLOW_DETAIL_SQL, requestParams.flowId);
                break;
            default:
                throw DatabaseException("unsupported type!");
        }
        if (!resultSet->Next()) {
            return false;
        }
        flowBody.metaType = ENUM_TO_STR(processType).value_or("");
        flowBody.id = resultSet->GetString("id");
        flowBody.tid = resultSet->GetString("tid");
        flowBody.timestamp = resultSet->GetInt64("start");
        flowBody.depth = resultSet->GetInt32("depth");
        flowBody.duration = resultSet->GetInt64("dur");
        flowBody.name = stringCache[resultSet->GetString("name")];
        if (processType == PROCESS_TYPE::CANN_API) {
            flowBody.pid = resultSet->GetString("pid");
        } else {
            flowBody.pid = flowBody.metaType;
            flowBody.rankId = resultSet->GetString("deviceId");
        }
        return true;
    };

    static bool QueryFlowCategoryEvents(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                        FlowCategoryEventsParams &params, PROCESS_TYPE processType,
                                        std::map<std::string, std::vector<FlowEventLocation>> &flowEventMap)
    {
        std::string sql;
        std::unique_ptr<SqliteResultSet> resultSet;
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                sql = "select name, streamId as tid, startNs as start, endNs - startNs as dur,\n"
                      " main.connectionId, depth, deviceId from TASK main join COMPUTE_TASK_INFO info\n"
                      " on main.globalTaskId = info.globalTaskId where startNs >= ? and endNs <= ? and deviceId = ?;";
                resultSet = ExecuteQuery(stmt, sql, params.startTime, params.endTime, params.rankId);
                break;
            case PROCESS_TYPE::HCCL:
                sql = "with tasks as (select startNs as start, endNs - startNs as dur,\n"
                      " ROWID as id, globalTaskId, depth, deviceId, connectionId from TASK "
                      " where startNs >= ? and endNs <= ? and deviceId = ?) "
                      " select name, planeId as tid,  start, dur, connectionId, depth, deviceId from tasks main "
                      " join COMMUNICATION_TASK_INFO info on main.globalTaskId = info.globalTaskId UNION "
                      " select opName as name, op.groupName||'group' as tid, op.startNs as start, "
                      " op.endNs - op.startNs as dur, tasks.connectionId, 0 as depth,deviceId from COMMUNICATION_OP op "
                      " join tasks on op.connectionId = tasks.connectionId group by op.opId;";
                resultSet = ExecuteQuery(stmt, sql, params.startTime, params.endTime, params.rankId);
                break;
            case PROCESS_TYPE::CANN_API:
            case PROCESS_TYPE::API:
                sql = " select name, type as tid, api.startNs as start, api.endNs - api.startNs as dur,"
                      " api.connectionId, api.depth, globalTid as pid from " + TABLE_CANN_API + " api join TASK"
                      " on TASK.connectionId = api.connectionId where TASK.startNs >= ? "
                      " and TASK.endNs <= ? and deviceId = ? group by api.connectionId";
                resultSet = ExecuteQuery(stmt, sql, params.startTime, params.endTime, params.rankId);
                break;
            default:
                throw DatabaseException("unsupported type!");
        }
        while (resultSet->Next()) {
            FlowEventLocation location;
            location.tid = resultSet->GetString("tid");
            location.timestamp = resultSet->GetInt64("start");
            location.depth = resultSet->GetInt32("depth");
            if (processType == PROCESS_TYPE::CANN_API) {
                location.pid = resultSet->GetString("pid");
            } else {
                location.pid = ENUM_TO_STR(processType).value_or("");
            }
            flowEventMap[resultSet->GetString("connectionId")].emplace_back(location);
        }
        return true;
    };

    static std::unique_ptr<SqliteResultSet> QueryUnitCounter(std::unique_ptr<SqlitePreparedStatement> &stmt,
          const Protocol::UnitCounterParams &requestParams, uint64_t minTimestamp)
    {
        auto processType = GetProcessType(requestParams.metaType);
        switch (processType) {
            case PROCESS_TYPE::HBM:
                return ExecuteQuery(stmt, HBM_UNIT_COUNTER_SQL, minTimestamp, requestParams.rankId,
                                    requestParams.processName, requestParams.startTime, requestParams.endTime);
            case PROCESS_TYPE::LLC:
                return ExecuteQuery(stmt, LLC_UNIT_COUNTER_SQL, minTimestamp, requestParams.processName,
                                    requestParams.rankId, requestParams.startTime, requestParams.endTime);
            case PROCESS_TYPE::DDR:
                return ExecuteQuery(stmt, DDR_UNIT_COUNTER_SQL, minTimestamp, requestParams.processName,
                                    requestParams.rankId, requestParams.startTime, requestParams.endTime);
            case PROCESS_TYPE::STARS_SOC:
                return ExecuteQuery(stmt, SOC_UNIT_COUNTER_SQL, minTimestamp, requestParams.processName,
                                    requestParams.rankId, requestParams.startTime, requestParams.endTime);
            case PROCESS_TYPE::ACC_PMU:
                return ExecuteQuery(stmt, PMU_UNIT_COUNTER_SQL, requestParams.processName, minTimestamp,
                                    requestParams.rankId, requestParams.startTime, requestParams.endTime);
            case PROCESS_TYPE::NPU_MEM:
                return ExecuteQuery(stmt, NPU_UNIT_COUNTER_SQL, requestParams.processName, minTimestamp,
                                    requestParams.rankId, requestParams.startTime, requestParams.endTime);
            default:
                throw DatabaseException("unsupported type!");
        }
    };

    static std::unique_ptr<SqliteResultSet> QueryThreadDetail(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                              const Protocol::ThreadDetailParams &requestParams,
                                                              uint64_t minTimestamp)
    {
        std::string sql;
        auto processType = GetProcessType(requestParams.metaType);
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                sql = "SELECT main.ROWID as id, startNs, endNs - startNs as duration,"
                      " depth, coalesce(CTI.name, main.taskType) as name FROM " + TABLE_TASK + " main "
                      " left join " + TABLE_COMPUTE_TASK_INFO + " CTI on CTI.globalTaskId = main.globalTaskId"
                      " WHERE main.ROWID = ?";
                return ExecuteQuery(stmt, sql, requestParams.id);
            case PROCESS_TYPE::HCCL:
                sql = " with tmp as (select main.globalTaskId, startNs, endNs, info.name,info.planeId,main.ROWID as id,"
                      " info.opId from " + TABLE_TASK + " main join " + TABLE_COMMUNICATION_TASK_INFO +
                      " info on info.globalTaskId = main.globalTaskId where main.deviceId = ?), "
                      " sub as (select ROWID as id,startNs,endNs-startNs as duration,opName as name,"
                      " groupName||'group' as tid, endNs from " + TABLE_COMMUNICATION_OP +
                      " where opId in (select opId from tmp group by opId) "
                      " UNION select id, startNs, endNs-startNs as duration, name, planeId||'' as tid,"
                      " endNs from tmp) select id, startNs, duration, 0 as depth, name from sub "
                      " where tid = ? AND id = ?";
                return ExecuteQuery(stmt, sql, requestParams.rankId, requestParams.tid, requestParams.id);
            case PROCESS_TYPE::CANN_API:
            case PROCESS_TYPE::API:
                sql = "select ROWID as id, startNs, endNs-startNs as duration, depth, name "
                      " from " + requestParams.metaType + " where ROWID = ? and startNs = ?";
                return ExecuteQuery(stmt, sql, requestParams.id, requestParams.startTime + minTimestamp);
            default:
                throw DatabaseException("unsupported type!");
        }
    };

static void QueryTaskInfoById(std::unique_ptr<SqlitePreparedStatement> &stmt,
             const Protocol::ThreadDetailParams &requestParams,
             Protocol::UnitThreadDetailBody &responseBody, std::map<std::string, std::string> &stringCache);

static std::unique_ptr<SqliteResultSet> QueryTaskStrInfoById(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                const Protocol::ThreadDetailParams &requestParams);

static std::unique_ptr<SqliteResultSet> QueryTaskCacheInfoById(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                    const Protocol::ThreadDetailParams &requestParams, bool attrInfoExist);
static bool isAttrInfoExist(std::unique_ptr<SqlitePreparedStatement> &stmt);

    static  std::unique_ptr<SqliteResultSet> QueryThreadTraces(
            std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::UnitThreadTracesParams &requestParams,
            uint64_t minTimestamp)
    {
        std::string sql;
        auto processType = GetProcessType(requestParams.metaType);
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                sql = "SELECT main.ROWID as id, startNs - ? as start_time, endNs - startNs as duration,"
                      " coalesce(c.name, main.taskType) as name, depth, ROUND(startNs / ?) as rank FROM TASK main "
                      " left join COMPUTE_TASK_INFO c "
                      " on c.globalTaskId = main.globalTaskId where deviceId = ? and streamId = ?"
                      " and start_time + duration >= ? AND start_time < ?"
                      " GROUP BY depth, rank, duration HAVING max(start_time) ORDER BY depth, start_time;";
                return ExecuteQuery(stmt, sql, minTimestamp, requestParams.timePerPx, requestParams.cardId,
                                    requestParams.threadId, requestParams.startTime, requestParams.endTime);
            case PROCESS_TYPE::HCCL:
                sql = "with tmp as (select main.ROWID, * from " + TABLE_TASK +
                  " main join " + TABLE_COMMUNICATION_TASK_INFO +
                  " info on info.globalTaskId = main.globalTaskId where main.deviceId = ?), "
                  " sub as (select startNs,endNs-startNs as duration,opName as name,groupName||'group' as tid,endNs,"
                  " ROWID as id from " + TABLE_COMMUNICATION_OP + " where opId in (select opId from tmp group by opId) "
                  " UNION select startNs,endNs-startNs as duration,name, planeId||'' as tid, endNs, "
                  " tmp.ROWID as id from tmp) select id, startNs-? as start_time,duration, name, 0 as depth, "
                  " ROUND(startNs / ?) as rank from sub "
                  " where tid = ? and start_time + duration >= ? AND start_time < ? "
                  " GROUP BY rank, duration HAVING max(start_time) ORDER BY start_time;";
                return ExecuteQuery(stmt, sql, requestParams.cardId, minTimestamp, requestParams.timePerPx,
                                    requestParams.threadId, requestParams.startTime, requestParams.endTime);
            case PROCESS_TYPE::CANN_API:
                sql = "select name, ROWID as id, startNs - ? as start_time, endNs - startNs as duration, depth,"
                      " ROUND(startNs / ?) as rank from " + requestParams.metaType + " a "
                      " where type = ? and globalTid = ? and start_time + duration >= ? AND start_time < ? "
                      " GROUP BY depth, rank, duration HAVING max(start_time) ORDER BY depth, start_time;";
                return ExecuteQuery(stmt, sql, minTimestamp, requestParams.timePerPx, requestParams.threadId,
                                    requestParams.processId, requestParams.startTime, requestParams.endTime);
            case PROCESS_TYPE::API:
                sql = "select name, ROWID as id, startNs - ? as start_time, endNs - startNs as duration, depth,"
                      " ROUND(startNs / ?) as rank from " + requestParams.metaType + " a "
                      " where globalTid = ? and start_time + duration >= ? AND start_time < ? "
                      " GROUP BY depth, rank, duration HAVING max(start_time) ORDER BY depth, start_time;";
                return ExecuteQuery(stmt, sql, minTimestamp, requestParams.timePerPx,
                                    requestParams.processId, requestParams.startTime, requestParams.endTime);
            default:
                throw DatabaseException("unsupported type!");
        }
    };

    static  std::unique_ptr<SqliteResultSet> QueryThreadTracesSummary(
            std::unique_ptr<SqlitePreparedStatement> &stmt,
            const Protocol::UnitThreadTracesSummaryParams &requestParams, uint64_t minTimestamp)
    {
        std::string sql;
        auto processType = GetProcessType(requestParams.metaType);
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                sql = "SELECT startNs - ? as start_time, endNs - startNs as duration, endNs - ? as end_time "
                      "FROM " + TABLE_TASK + " WHERE deviceId = ? AND start_time >= ? "
                      "AND start_time <= ? AND depth = 0 ORDER BY startNs;";
                return ExecuteQuery(stmt, sql, minTimestamp, minTimestamp, requestParams.cardId,
                                    requestParams.startTime, requestParams.endTime);
            case PROCESS_TYPE::HCCL:
                sql = "SELECT startNs - ? as start_time, endNs - startNs as duration, endNs - ? as end_time "
                      "FROM " + TABLE_TASK + " main join " + TABLE_COMMUNICATION_TASK_INFO + " info "
                      " on main.globalTaskId = info.globalTaskId"
                      " WHERE deviceId = ? AND start_time >= ? AND start_time <= ? ORDER BY startNs;";
                return ExecuteQuery(stmt, sql, minTimestamp, minTimestamp, requestParams.cardId,
                                    requestParams.startTime, requestParams.endTime);
            case PROCESS_TYPE::CANN_API:
            case PROCESS_TYPE::API:
                sql = "SELECT startNs - ? as start_time, endNs - startNs as duration, endNs - ? as end_time "
                      " FROM " + TABLE_CANN_API;
                if (DataBaseManager::Instance().GetFileType() == FileType::PYTORCH) {
                    sql += " UNION SELECT startNs -? as start_time,endNs - startNs as duration,"
                           " endNs - ? as end_time from " + TABLE_API;
                }
                sql += " WHERE globalTid = ? AND start_time >= ? AND start_time <= ? ORDER BY start_time;";
                return ExecuteQuery(stmt, sql, minTimestamp, minTimestamp, requestParams.processId,
                                    requestParams.startTime, requestParams.endTime);
            default:
                throw DatabaseException("unsupported type!");
        }
    };

    static  void CalculateSelfTime(std::vector<Protocol::SimpleSlice> &rows,
                                   std::map<std::string, uint64_t> &selfTimeKeyValue,
                                   uint64_t startTime, uint64_t endTime)
    {
        int32_t i = 0;
        int32_t j = 0;
        if (rows.empty()) {
            Server::ServerLog::Error("simpleSlice array size is zero!");
            return;
        }
        uint64_t tmpSelfTime = rows.at(0).duration;
        while (i < rows.size()) {
            // j滑完直接滑完所有i
            if (j == rows.size()) {
                // 处理当前tmpSelfTime
                AddData(selfTimeKeyValue, rows.at(i).name, tmpSelfTime);
                // 处理剩余元素
                DealLastData(rows, selfTimeKeyValue, startTime, endTime, i);
                break;
            }
            Protocol::SimpleSlice rowI = rows.at(i);
            Protocol::SimpleSlice rowJ = rows.at(j);
            // 层数相等 or 同一元素, j右移
            if (rowI.depth == rowJ.depth || i >= j) {
                j++;
                continue;
            }
            // rows[i]不属于框选范围内，跳过
            if (rows.at(i).timestamp > endTime || rows.at(i).endTime < startTime) {
                if (i + 1 == rows.size()) { // i滑完结束
                    break;
                }
                i++;
                tmpSelfTime = rows.at(i).duration;
                continue;
            }
            // j元素超出i元素覆盖范围，或者j右移到下一层, 记录i元素selfTime并i右移(隐式|| rowJ.timestamp < rowI.timestamp)
            if (rowJ.endTime > rowI.endTime || rowI.depth + 1 < rowJ.depth) {
                AddData(selfTimeKeyValue, rowI.name, tmpSelfTime);
                if (i + 1 == rows.size()) { // i滑完结束
                    break;
                }
                i++;
                tmpSelfTime = rows.at(i).duration;
                continue;
            }
            // 符合要求的元素
            if (rowJ.timestamp >= rowI.timestamp && rowJ.endTime <= rowI.endTime) {
                tmpSelfTime -= rowJ.duration;
            }
            j++;
        }
    }

    static  void ReduceThread(const std::vector<Protocol::SimpleSlice> &rows,
                             const std::map<std::string, uint64_t> &selfTimeKeyValue,
                             Protocol::UnitThreadsBody &responseBody)
    {
        for (auto &cur : rows) {
            int index = -1;
            for (int i = 0; i < responseBody.data.size(); i++) {
                if (responseBody.data[i].title == cur.name) {
                    index = i;
                    break;
                }
            }
            if (index == -1) {
                Protocol::Threads threads {};
                threads.title = cur.name;
                threads.wallDuration = cur.duration;
                threads.occurrences = 1;
                threads.avgWallDuration = cur.duration;
                threads.selfTime = selfTimeKeyValue.at(cur.name);
                responseBody.data.emplace_back(threads);
            } else {
                responseBody.data[index].wallDuration += cur.duration;
                responseBody.data[index].occurrences += 1;
                responseBody.data[index].avgWallDuration =
                        responseBody.data[index].wallDuration / responseBody.data[index].occurrences;
            }
        }
    }

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
    static inline std::unique_ptr<SqliteResultSet> ExecuteQuery(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                                const std::string &sql, Args&&... args)
    {
        if (stmt == nullptr) {
            throw DatabaseException("Failed to prepare sql.");
        }
        if (!stmt->Prepare(sql)) {
            throw DatabaseException("Failed to prepare sql.");
        }
        stmt->Reset();
        stmt->BindParams(std::forward<Args>(args)...);
        auto result = stmt->ExecuteQuery();
        return result;
    };

    static inline PROCESS_TYPE GetProcessType(const std::string &metaType)
    {
        auto processType = STR_TO_ENUM<PROCESS_TYPE>(metaType);
        if (!processType.has_value()) {
            return static_cast<PROCESS_TYPE>(atoi(metaType.c_str()));
        }
        return processType.value();
    }

private:
    static inline bool DealLastData(std::vector<Protocol::SimpleSlice> &rows,
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

    static  void QueryExtremumTimeOfFirstDepth(
            std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::UnitThreadsParams &requestParams,
            uint64_t minTimestamp, Protocol::ExtremumTimestamp &extremumTimestamp)
    {
        std::string sql;
        uint64_t startTime = requestParams.startTime + minTimestamp;
        uint64_t endTime = requestParams.endTime + minTimestamp;
        auto processType = GetProcessType(requestParams.metaType);
        std::unique_ptr<SqliteResultSet> resultSet;
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                sql = "select min(startNs) as minStart, max(endNs) AS maxEnd from " + TABLE_TASK +
                      " where deviceId = ? and streamId = ? and endNs >= ? AND startNs <= ?"
                      " AND depth = 0";
                resultSet = ExecuteQuery(stmt, sql, requestParams.rankId, requestParams.tid,
                                         startTime, endTime);
                break;
            case PROCESS_TYPE::HCCL:

            default:
                throw DatabaseException("unsupported type!");
        }
        while (resultSet->Next()) {
            extremumTimestamp.minTimestamp = floor(resultSet->GetDouble("minStart"));
            extremumTimestamp.maxTimestamp = ceil(resultSet->GetDouble("maxEnd"));
        }
        stmt->Reset();
    }
};
}

#endif // PROFILER_SERVER_TRACEDATABASEHELPER_H