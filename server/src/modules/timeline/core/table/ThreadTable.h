/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_THREADTABLE_H
#define PROFILER_SERVER_THREADTABLE_H
#include "TextTableColum.h"
#include "Table.h"
namespace Dic::Module::Timeline {
struct ThreadPO {
    uint64_t trackId = 0;
    std::string tid;
    std::string pid;
    std::string threadName;
    uint64_t threadSortIndex = 0;
};
class ThreadTable : public Table<ThreadPO> {
public:
    ThreadTable() = default;
    ~ThreadTable() = default;

protected:
    const std::unordered_map<std::string_view, assign>& GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { ThreadColumn::TRACK_ID, TrackIdHandle },
            { ThreadColumn::TID, TidHandle },
            { ThreadColumn::PID, PidHandle },
            { ThreadColumn::THREAD_NAME, ThreadNameHandle },
            { ThreadColumn::THREAD_SORT_INDEX, ThreadSortIndexHandle } };

        return assignMap;
    }
    const std::string &GetTableName() override
    {
        static std::string tableName = "thread";
        return tableName;
    }
    static void TrackIdHandle(ThreadPO &threadPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void TidHandle(ThreadPO &threadPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void PidHandle(ThreadPO &threadPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void ThreadNameHandle(ThreadPO &threadPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void ThreadSortIndexHandle(ThreadPO &threadPO, const std::unique_ptr<SqliteResultSet> &resultSet);
};
}
#endif // PROFILER_SERVER_THREADTABLE_H
