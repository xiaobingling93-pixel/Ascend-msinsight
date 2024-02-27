/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACEDATABASEHELPER_H
#define PROFILER_SERVER_TRACEDATABASEHELPER_H

#include "TableDefs.h"
#include "SqlitePreparedStatement.h"
#include "VirtualTraceDatabase.h"

namespace Dic::Module::Timeline {
enum class PROCESS_TYPE {
    ASCEND_HARDWARE,
    HCCL,
    CANN
};

class TraceDatabaseHelper {
public:
    static inline std::unique_ptr<SqliteResultSet> QueryThreadsByPid(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                                     const Protocol::UnitThreadsParams &requestParams,
                                                                     uint64_t minTimestamp)
    {
        std::string sql;
        auto processType = static_cast<PROCESS_TYPE>(atoi(requestParams.pid.c_str()));
        Protocol::ExtremumTimestamp extremumTimestamp {};
        auto resultSet = QueryExtremumTimeOfFirstDepth(stmt, requestParams, minTimestamp);
        while (resultSet->Next()) {
            extremumTimestamp.minTimestamp = resultSet->GetUint64("minStart");
            extremumTimestamp.maxTimestamp = resultSet->GetUint64("maxEnd");
        }
        stmt->Reset();
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                sql = "select start, end - start as duration, end, coalesce(d.value, e.value) as name, depth "
                      " from " + TABLE_TASK + " main "
                      " left join (select name, correlationId, planeId as tid from " + TABLE_COMMUNICATION_TASK_INFO +
                    "                 UNION select name, correlationId, null as tid from " + TABLE_COMPUTE_TASK_INFO +
                    " ) c on c.correlationId = main.correlationId"
                    "    left JOIN " + TABLE_STRING_IDS + " d on d.id = c.name"
                    "    left JOIN " + TABLE_STRING_IDS + " e on e.id = main.taskType"
                    " where deviceId = ? and streamId = ? and end >= ? AND start <= ?"
                    " ORDER BY depth ASC, start ASC;";
                resultSet = ExecuteQuery(stmt, sql, requestParams.rankId, requestParams.tid,
                                         extremumTimestamp.minTimestamp, extremumTimestamp.maxTimestamp);
                break;
            case PROCESS_TYPE::HCCL:
            case PROCESS_TYPE::CANN:
            default:
                throw DatabaseException("unsupported type!");
        }
        return resultSet;
    };

    static inline std::unique_ptr<SqliteResultSet> QueryThreadDetail(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                                     const Protocol::ThreadDetailParams &requestParams,
                                                                     uint64_t minTimestamp)
    {
        std::string sql;
        auto processType = static_cast<PROCESS_TYPE>(atoi(requestParams.pid.c_str()));
        std::unique_ptr<SqliteResultSet> resultSet;
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                sql = "SELECT main.correlationId as id, start, end - start as duration,"
                      " depth, coalesce(CTI.name, main.taskType) as name, CTI.*"
                      " FROM " + TABLE_TASK + " main "
                      " left join COMPUTE_TASK_INFO CTI on main.correlationId = CTI.correlationId"
                      " WHERE depth = ? AND deviceId = ? AND streamId = ? AND  abs(start - ?) < 1e-5";
                resultSet = ExecuteQuery(stmt, sql, requestParams.depth, requestParams.rankId,
                                         requestParams.tid, requestParams.startTime + minTimestamp);
                break;
            case PROCESS_TYPE::HCCL:
            case PROCESS_TYPE::CANN:
            default:
                throw DatabaseException("unsupported type!");
        }
        return resultSet;
    };

    static inline std::unique_ptr<SqliteResultSet> QueryThreadTraces(
            std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::UnitThreadTracesParams &requestParams,
            uint64_t minTimestamp)
    {
        std::string sql;
        auto processType = static_cast<PROCESS_TYPE>(atoi(requestParams.processId.c_str()));
        std::unique_ptr<SqliteResultSet> resultSet;
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                sql = "SELECT main.correlationId as id, start - ? as start_time, end - start as duration,"
                      " coalesce(c.name, main.taskType) as name, depth, "
                      " ROUND(start / ? ) as rank FROM TASK main "
                      " left join (select name, correlationId, planeId as tid from COMMUNICATION_TASK_INFO"
                      "     UNION select name, correlationId, null as tid from COMPUTE_TASK_INFO) c "
                      " on c.correlationId = main.correlationId"
                      " where deviceId = ? and streamId = ? and start_time + duration >= ? AND start_time < ?"
                      " GROUP BY depth, rank HAVING max(start)"
                      " ORDER BY depth, start_time;";
                resultSet = ExecuteQuery(stmt, sql, minTimestamp, requestParams.timePerPx, requestParams.cardId,
                                         requestParams.threadId, requestParams.startTime, requestParams.endTime);
                break;
            case PROCESS_TYPE::HCCL:
            case PROCESS_TYPE::CANN:
            default:
                throw DatabaseException("unsupported type!");
        }
        return resultSet;
    };

    static inline std::unique_ptr<SqliteResultSet> QueryThreadTracesSummary(
            std::unique_ptr<SqlitePreparedStatement> &stmt,
            const Protocol::UnitThreadTracesSummaryParams &requestParams, uint64_t minTimestamp)
    {
        std::string sql;
        auto processType = static_cast<PROCESS_TYPE>(atoi(requestParams.processId.c_str()));
        std::unique_ptr<SqliteResultSet> resultSet;
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                sql = "SELECT start - ? as start_time, end - start as duration, end - ? as end_time "
                      "FROM " + TABLE_TASK + " WHERE deviceId = ? AND start_time >= ? AND start_time <= ? "
                      "AND depth = 0 ORDER BY start;";
                resultSet = ExecuteQuery(stmt, sql, minTimestamp, minTimestamp, requestParams.cardId,
                                         requestParams.startTime, requestParams.endTime);
                break;
            case PROCESS_TYPE::HCCL:
            case PROCESS_TYPE::CANN:
            default:
                throw DatabaseException("unsupported type!");
        }
        return resultSet;
    };

    static inline void CalculateSelfTime(std::vector<Protocol::SimpleSlice> &rows,
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

    static inline void ReduceThread(const std::vector<Protocol::SimpleSlice> &rows,
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

    static inline std::unique_ptr<SqliteResultSet> QueryExtremumTimeOfFirstDepth(
            std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::UnitThreadsParams &requestParams,
            uint64_t minTimestamp)
    {
        std::string sql;
        uint64_t startTime = requestParams.startTime + minTimestamp;
        uint64_t endTime = requestParams.endTime + minTimestamp;
        auto processType = static_cast<PROCESS_TYPE>(atoi(requestParams.pid.c_str()));
        std::unique_ptr<SqliteResultSet> resultSet;
        switch (processType) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                sql = "select min(start) as minStart, max(end) AS maxEnd from " + TABLE_TASK +
                      " where deviceId = ? and streamId = ? and end >= ? AND start <= ?"
                      " AND depth = 0";
                resultSet = ExecuteQuery(stmt, sql, requestParams.rankId, requestParams.tid,
                                         startTime, endTime);
                break;
            case PROCESS_TYPE::HCCL:

            case PROCESS_TYPE::CANN:

            default:
                throw DatabaseException("unsupported type!");
        }
        return resultSet;
    }
};
}


#endif // PROFILER_SERVER_TRACEDATABASEHELPER_H
