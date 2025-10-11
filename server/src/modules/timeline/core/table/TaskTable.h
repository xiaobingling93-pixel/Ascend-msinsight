/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef PROFILER_SERVER_TASKTABLE_H
#define PROFILER_SERVER_TASKTABLE_H
#include "TextTableColum.h"
#include "Table.h"
namespace Dic::Module::Timeline {
struct TaskPO {
    uint64_t id = 0;
    uint64_t timestamp = 0;
    uint64_t endTime = 0;
    uint64_t deviceId = 0;
    // db的数据类型是INTEGER，可能存在负值
    int64_t connectionId = 0;
    uint64_t globalTaskId = 0;
    uint64_t globalPid = 0;
    uint64_t taskType = 0;
    uint64_t contextId = 0;
    uint64_t streamId = 0;
    uint64_t taskId = 0;
    uint64_t modelId = 0;
    uint64_t domainId = 0;
};
class TaskTable : public Table<TaskPO> {
public:
    TaskTable() = default;
    virtual ~TaskTable() = default;

protected:
    const std::unordered_map<std::string_view, assign> &GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { TaskColumn::ROW_ID, IdHandle },
            { TaskColumn::TIMESTAMP, TimestampHandle },
            { TaskColumn::ENDTIME, EndTimeHandle },
            { TaskColumn::DECICED_ID, DeviceIdHandle },
            { TaskColumn::CONNECTION_ID, ConnectionIdHandle },
            { TaskColumn::GLOBAL_TASK_ID, GlobalTaskIdHandle },
            { TaskColumn::GLOBAL_PID, GlobalPidHandle },
            { TaskColumn::TASK_TYPE, TaskTypeHandle },
            { TaskColumn::CONTEXT_ID, ContextIdHandle },
            { TaskColumn::STREAM_ID, StreamIdHandle },
            { TaskColumn::TASK_ID, TaskIdHandle },
            { TaskColumn::MODEL_ID, ModelIdHandle } };

        return assignMap;
    }
    const std::string &GetTableName() override
    {
        static std::string tableName = "TASK";
        return tableName;
    }
    static void IdHandle(TaskPO &taskPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void TimestampHandle(TaskPO &taskPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void EndTimeHandle(TaskPO &taskPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void DeviceIdHandle(TaskPO &taskPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void ConnectionIdHandle(TaskPO &taskPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void GlobalTaskIdHandle(TaskPO &taskPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void GlobalPidHandle(TaskPO &taskPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void TaskTypeHandle(TaskPO &taskPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void ContextIdHandle(TaskPO &taskPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void StreamIdHandle(TaskPO &taskPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void TaskIdHandle(TaskPO &taskPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void ModelIdHandle(TaskPO &taskPO, const std::unique_ptr<SqliteResultSet> &resultSet);
};
}
#endif // PROFILER_SERVER_TASKTABLE_H
