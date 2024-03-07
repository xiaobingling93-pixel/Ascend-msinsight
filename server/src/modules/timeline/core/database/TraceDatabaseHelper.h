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

namespace Dic::Module::Timeline {
using namespace Protocol;
const std::string HBM_UNIT_COUNTER_SQL = "select timestampNs-? as startTime,hbmId||'/'|| case when type=1 then 'Read' "
         " else 'Write' end as name,  case when type=1 then '{\"Read(MB/s)\":' || bandwidth || '}' "
         " else '{\"Write(MB/s)\":' || bandwidth || '}' end as args from HBM main "
         " where device_id= ? and name = ? AND startTime >= ? AND startTime <= ? ORDER BY timestampNs ASC";

const std::string LLC_UNIT_COUNTER_SQL = "with main as (select timestampNs - ? as startTime, "
         " format('%s %s', llcId, case when mode=1 then "
         "'Read' else 'Write' end) as modeName,? as processName,throughput,hitRate from LLC where deviceId = ?"
         " AND startTime >= ? AND startTime <= ? and glob(modeName||'*', processName)) select startTime, "
         " case when glob('*Throughput', processName) then format('{\"Throughput(MB/s)\":%s}', throughput) "
         " else format('{\"Hit Rate(%%)\":%s}', hitRate) end as args from main ORDER BY startTime ASC";
const std::string DDR_UNIT_COUNTER_SQL = "select timestampNs-? as startTime, case when ? = 'Read' "
         " then format('{\"Read(MB/s)\":%s}', read) else format('{\"Write(MB/s)\":%s}', write) end as args from DDR "
         " where deviceId = ? and startTime >= ? AND startTime <= ? ORDER BY startTime ASC";
const std::string SOC_UNIT_COUNTER_SQL = "select timestampNs - ? as startTime, case when ? = 'L2 Buffer Bw Level' then "
         " format('{\"L2 Buffer Bw Level\":%s}', l2BufferBwLevel) else "
         " format('{\"Mata Bw Level\":%s}', mataBwLevel) end as args from SOC_BANDWIDTH_LEVEL"
         " where deviceId = ? and startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
const std::string PMU_UNIT_COUNTER_SQL = "with pn as (select ? as value) select timestampNs - ? as startTime, "
        " format('{\"value\":%s, \"acc_id\":%s}', case when pn.value='read_bandwidth' then readBandwidth "
        " when pn.value = 'write_bandwidth' then writeBandwidth when pn.value = 'read_ost' then readOst "
        " else writeOst end, accId) as args  from ACC_PMU join pn "
        " where deviceId = ? and startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";
const std::string NPU_UNIT_COUNTER_SQL = "with pn as (select ? as value) select timestampNs - ? as startTime, "
        " case type when 0 then 'APP*'  else 'Device*' end as typeName, format('{\"KB\":%s}',"
        " case when glob('*DDR', pn.value) then ddrUsage when glob('*HBM', pn.value) then "
        " hbmUsage else hbmUsage + ddrUsage end) as args from NPU_MEM join pn where deviceId = ? and "
        " glob(typeName, pn.value) and startTime >= ? AND startTime <= ? ORDER BY startTime ASC;";

class TraceDatabaseHelper {
public:
    static std::unique_ptr<SqliteResultSet> QueryThreadsByPid(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                              const Protocol::UnitThreadsParams &requestParams,
                                                              uint64_t minTimestamp)
    {
        std::string sql;
        auto processType = GetProcessType(requestParams.pid, requestParams.rankId);
        Protocol::ExtremumTimestamp extremumTimestamp {};
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                QueryExtremumTimeOfFirstDepth(stmt, requestParams, minTimestamp, extremumTimestamp);
                sql = "select startNs,endNs - startNs as duration,endNs,coalesce(c.name, main.taskType) as name, depth "
                      " from " + TABLE_TASK + " main "
                      " left join (select name, globalTaskId, planeId as tid from " + TABLE_COMMUNICATION_TASK_INFO +
                    "                 UNION select name, globalTaskId, null as tid from " + TABLE_COMPUTE_TASK_INFO +
                    " ) c on c.globalTaskId = main.globalTaskId"
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
                      " where groupName = ?) select * from sub where sub.endNs >= ? "
                      " and sub.startNs <= ?;";
                return ExecuteQuery(stmt, sql, requestParams.rankId, requestParams.tid, requestParams.tid,
                                    requestParams.startTime + minTimestamp, requestParams.endTime + minTimestamp);
            case PROCESS_TYPE::HOST:
                sql = "select startNs, endNs - startNs as duration, endNs, name, depth from " + TABLE_API + " main "
                      " where type = ? and globalTid = ? and depth = 0 and endNs >= ? AND startNs <= ?"
                      " ORDER BY depth ASC, startNs ASC;";
                return ExecuteQuery(stmt, sql, requestParams.tid, requestParams.pid,
                                    requestParams.startTime + minTimestamp, requestParams.endTime + minTimestamp);
            default:
                throw DatabaseException("unsupported type!");
        }
    };

    static std::unique_ptr<SqliteResultSet> QueryUnitCounter(std::unique_ptr<SqlitePreparedStatement> &stmt,
          const Protocol::UnitCounterParams &requestParams, uint64_t minTimestamp)
    {
        auto processType = GetProcessType(requestParams.pid, requestParams.rankId);
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
        auto processType = GetProcessType(requestParams.pid, requestParams.rankId);
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                sql = "SELECT main.globalTaskId as id, startNs, endNs - startNs as duration,"
                      " depth, coalesce(CTI.name, main.taskType) as name FROM " + TABLE_TASK + " main "
                      " left join (select name, globalTaskId, planeId as tid from " + TABLE_COMMUNICATION_TASK_INFO +
                      "                 UNION select name, globalTaskId, null as tid from " + TABLE_COMPUTE_TASK_INFO +
                      " ) CTI on CTI.globalTaskId = main.globalTaskId"
                      " WHERE depth = ? AND deviceId = ? AND streamId = ? AND startNs = ?";
                return ExecuteQuery(stmt, sql, requestParams.depth, requestParams.rankId,
                                    requestParams.tid, requestParams.startTime + minTimestamp);
            case PROCESS_TYPE::HCCL:
                sql = " with tmp as (select main.globalTaskId, startNs, endNs, info.name, info.planeId,"
                      " info.opId from " + TABLE_TASK + " main join " + TABLE_COMMUNICATION_TASK_INFO +
                      " info on info.globalTaskId = main.globalTaskId where main.deviceId = ?), "
                      " sub as (select opId as id,startNs,endNs-startNs as duration,opName as name,groupName as tid, "
                      " endNs from " + TABLE_COMMUNICATION_OP + " where opId in (select opId from tmp group by opId)\n "
                      " UNION select globalTaskId as id, startNs, endNs-startNs as duration, name, planeId as tid,"
                      " endNs from tmp) select id, startNs, duration, 0 as depth, name from sub "
                      " where tid = ? AND startNs = ?";
                return ExecuteQuery(stmt, sql, requestParams.rankId, requestParams.tid,
                                    requestParams.startTime + minTimestamp);
            case PROCESS_TYPE::HOST:
                sql = "select connectionId as id, startNs, endNs-startNs as duration, 0 as depth, name from API"
                      " where depth = ? and type = ? and globalTid = ? and startNs = ?";
                return ExecuteQuery(stmt, sql, requestParams.depth, requestParams.tid, requestParams.pid,
                                    requestParams.startTime + minTimestamp);
            default:
                throw DatabaseException("unsupported type!");
        }
    };

static std::unique_ptr<SqliteResultSet> QueryComputeTaskInfoById(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                          uint64_t id)
{
    std::string sql = "SELECT inputShapes, inputDataTypes, inputFormats, outputShapes, outputDataTypes, outputFormats"
                      "  FROM " + TABLE_COMPUTE_TASK_INFO + " where globalTaskId = ?";
    return ExecuteQuery(stmt, sql, id);
}

static std::unique_ptr<SqliteResultSet> QueryCommunicationTaskInfoById(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                          uint64_t id)
{
    std::string sql = "SELECT com.taskType,com.planeId, com.groupName,com.notifyId ,com.rdmaType,"
                      "com.srcRank,com.dstRank,com.transportType,com.size,com.dataType, "
                      "com.linkType ,com.opId "
                      "  FROM " + TABLE_COMMUNICATION_TASK_INFO + " com where globalTaskId = ?";
    return ExecuteQuery(stmt, sql, id);
}

    static  std::unique_ptr<SqliteResultSet> QueryThreadTraces(
            std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::UnitThreadTracesParams &requestParams,
            uint64_t minTimestamp)
    {
        std::string sql;
        auto processType = GetProcessType(requestParams.processId, requestParams.cardId);
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                sql = "SELECT main.globalTaskId as id, startNs - ? as start_time, endNs - startNs as duration,"
                      " coalesce(c.name, main.taskType) as name, depth, ROUND(startNs / ?) as rank FROM TASK main "
                      " left join (select name, globalTaskId, planeId as tid from COMMUNICATION_TASK_INFO"
                      "     UNION select name, globalTaskId, null as tid from COMPUTE_TASK_INFO) c "
                      " on c.globalTaskId = main.globalTaskId where deviceId = ? and streamId = ?"
                      " and start_time + duration >= ? AND start_time < ?"
                      " GROUP BY depth, rank, duration HAVING max(start_time) ORDER BY depth, start_time;";
                return ExecuteQuery(stmt, sql, minTimestamp, requestParams.timePerPx, requestParams.cardId,
                                    requestParams.threadId, requestParams.startTime, requestParams.endTime);
            case PROCESS_TYPE::HCCL:
                sql = "with tmp as (select * from " + TABLE_TASK + " main join " + TABLE_COMMUNICATION_TASK_INFO +
                  " info on info.globalTaskId = main.globalTaskId where main.deviceId = ?), "
                  " sub as (select startNs,endNs-startNs as duration,opName as name,groupName as tid,endNs,opId as id"
                  " from " + TABLE_COMMUNICATION_OP + " where opId in (select opId from tmp group by opId)\n UNION"
                  " select startNs,endNs-startNs as duration,name, planeId as tid, endNs, tmp.globalTaskId as id "
                  " from tmp) select id, startNs-? as start_time,duration, name, 0 as depth, "
                  " ROUND(startNs / ?) as rank from sub "
                  " where tid = ? and start_time + duration >= ? AND start_time < ? "
                  " GROUP BY rank, duration HAVING max(start_time) ORDER BY start_time;";
                return ExecuteQuery(stmt, sql, requestParams.cardId, minTimestamp, requestParams.timePerPx,
                                    requestParams.threadId, requestParams.startTime, requestParams.endTime);
            case PROCESS_TYPE::HOST:
                sql = "select name, connectionId as id, startNs - ? as start_time, endNs - startNs as duration, depth,"
                      " ROUND(startNs / ?) as rank from API a where type = ? and globalTid = ?"
                      " and start_time + duration >= ? AND start_time < ? "
                      " GROUP BY depth, rank, duration HAVING max(start_time) ORDER BY depth, start_time;";
                return ExecuteQuery(stmt, sql, minTimestamp, requestParams.timePerPx, requestParams.threadId,
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
        auto processType = GetProcessType(requestParams.processId, requestParams.cardId);
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
            case PROCESS_TYPE::HOST:
                sql = "SELECT startNs - ? as start_time, endNs - startNs as duration, endNs - ? as end_time "
                      "FROM " + TABLE_API + " main "
                      " WHERE globalTid = ? AND depth = 0 AND start_time >= ? AND start_time <= ? ORDER BY startNs;";
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

private:
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
        stmt->BindParams(std::forward<Args>(args)...);
        auto result = stmt->ExecuteQuery();
        return result;
    };

    static inline PROCESS_TYPE GetProcessType(const std::string &type, const std::string &cardId)
    {
        if (strcmp(cardId.c_str(), "Host") == 0) {
            return PROCESS_TYPE::HOST;
        }
        auto processType = STR_TO_ENUM<PROCESS_TYPE>(type);
        if (!processType.has_value()) {
            return static_cast<PROCESS_TYPE>(atoi(type.c_str()));
        }
        return processType.value();
    }

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
        auto processType = GetProcessType(requestParams.pid, requestParams.rankId);
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

            case PROCESS_TYPE::HOST:

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
