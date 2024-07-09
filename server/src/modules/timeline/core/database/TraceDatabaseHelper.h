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
/* Functions for BbTraceDataBase */
static std::optional<std::string> QueryConnectionId(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                    const Protocol::UnitFlowsParams &requestParams);
static std::unique_ptr<SqliteResultSet> QueryThreadsByPid(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                          const Protocol::UnitThreadsParams &requestParams,
                                                          const std::string &rankId, uint64_t minTimestamp);

static std::unique_ptr<SqliteResultSet> QueryUnitCounter(std::unique_ptr<SqlitePreparedStatement> &stmt,
      const Protocol::UnitCounterParams &requestParams, uint64_t minTimestamp, const std::string& rankId);

static std::optional<SliceDto> QueryThreadDetail(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                 const Protocol::ThreadDetailParams &requestParams);

static void QueryTaskInfoById(std::unique_ptr<SqlitePreparedStatement> &stmt,
             const Protocol::ThreadDetailParams &requestParams,
             Protocol::UnitThreadDetailBody &responseBody, std::map<std::string, std::string> &stringCache,
             std::string& metaVersion);

static std::unique_ptr<SqliteResultSet> QueryTaskStrInfoById(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                const Protocol::ThreadDetailParams &requestParams, std::string& metaVersion);

static std::unique_ptr<SqliteResultSet> QueryTaskCacheInfoById(std::unique_ptr<SqlitePreparedStatement> &stmt,
       const Protocol::ThreadDetailParams &requestParams, bool attrInfoExist, std::string& metaVersion);
static bool isAttrInfoExist(std::unique_ptr<SqlitePreparedStatement> &stmt);

static std::unique_ptr<SqliteResultSet> QuerySystemViewData(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                            const Protocol::SystemViewParams &requestParams,
                                                            const std::string& rankId);
static  std::unique_ptr<SqliteResultSet> QueryThreadTraces(
        std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::UnitThreadTracesParams &requestParams,
        const std::string& rankId, uint64_t minTimestamp);

static  std::unique_ptr<SqliteResultSet> QueryThreadTracesSummary(
        std::unique_ptr<SqlitePreparedStatement> &stmt,
        const Protocol::UnitThreadTracesSummaryParams &requestParams, const std::string& rankId, uint64_t minTimestamp);

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

    static  void ReduceThread(const std::vector<CompeteSliceDomain> &rows,
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
    static inline std::unique_ptr<SqliteResultSet> Execute(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                           Args&&... args)
    {
        stmt->BindParams(std::forward<Args>(args)...);
        auto result = stmt->ExecuteQuery();
        if (result == nullptr) {
            throw DatabaseException("Failed to ExecuteQuery.");
        }
        return result;
    };

    template <typename... Args>
    static inline std::unique_ptr<SqliteResultSet> ExecuteQuery(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                                const std::string &sql, Args&&... args)
    {
        Prepare(stmt, sql);
        return Execute(stmt, std::forward<Args>(args)...);
    };

    static inline std::unique_ptr<SqlitePreparedStatement>& Prepare(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                                   const std::string &sql)
    {
        if (stmt == nullptr) {
            throw DatabaseException("Failed to prepare sql.");
        }
        if (!stmt->Prepare(sql)) {
            throw DatabaseException("Failed to prepare sql.");
        }
        stmt->Reset();
        return stmt;
    };

    static inline PROCESS_TYPE GetProcessType(const std::string &metaType)
    {
        auto processType = STR_TO_ENUM<PROCESS_TYPE>(metaType);
        if (!processType.has_value()) {
            return static_cast<PROCESS_TYPE>(atoi(metaType.c_str()));
        }
        return processType.value();
    }
static std::unique_ptr<SqliteResultSet> QueryThreadSameOperatorsDetails(std::unique_ptr<SqlitePreparedStatement> &stmt,
     const Protocol::UnitThreadsOperatorsParams &requestParams, const std::string& rankId,
     uint64_t minTimestamp, const std::string& orderBy);
static bool QueryEventsViewData4Db(std::unique_ptr <SqlitePreparedStatement> &stmt,
    const Protocol::EventsViewParams &params, Protocol::EventsViewBody &body, uint64_t minTimestamp,
    const std::string& rankId);
/* Functions for JsonTraceDataBase */
static bool QueryEventsViewData4Text(std::unique_ptr <SqlitePreparedStatement> &stmt,
    const Protocol::EventsViewParams &params, Protocol::EventsViewBody &body, uint64_t minTimestamp);
static void QueryThreadTracesHelper(std::vector<Protocol::RowThreadTrace> &rowThreadTraceVec,
    const Protocol::UnitThreadTracesParams &requestParams, Protocol::UnitThreadTracesBody &responseBody);
static void QueryAllSliceInRangeByTrackIdHelper(std::unique_ptr<SqliteResultSet> &resultSet,
    uint64_t unitTime, uint64_t minTimestamp, Protocol::UnitThreadTracesSummaryBody &responseBody);
static void SetKernelDetailHelpler(std::unique_ptr<SqliteResultSet> resultSet, uint64_t minTimestamp,
                            Protocol::KernelDetailsBody &responseBody);
static void FilterTopLevelApi(std::vector<Protocol::FlowLocation> &originData, const std::set<std::string> &pattern,
    std::vector<Protocol::FlowLocation> &filterData, std::vector<uint32_t> &indexes);

private:
/* Functions for BbTraceDataBase */
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
};
}

#endif // PROFILER_SERVER_TRACEDATABASEHELPER_H