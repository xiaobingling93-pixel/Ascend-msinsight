/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include <string>
#include "TaskTable.h"
#include "TestCaseDatabaseUtil.h"
using namespace Dic::Protocol;
using namespace Dic::TimeLine::TestCaseUtil;
class TaskTableTest : public ::testing::Test {};

TEST_F(TaskTableTest, TestTaskTableColumnMaping)
{
    sqlite3 *db = nullptr;
    std::string sql = "CREATE TABLE TASK (startNs INTEGER,endNs INTEGER,deviceId INTEGER,connectionId "
        "INTEGER,globalTaskId INTEGER,globalPid INTEGER,taskType INTEGER,contextId INTEGER,streamId "
        "INTEGER,taskId INTEGER,modelId INTEGER, depth integer);";
    TestCaseDatabaseUtil::CreateDatabse(db, sql);
    std::string sqlInsert = "INSERT INTO TASK (startNs, endNs, deviceId, connectionId, globalTaskId, globalPid, "
        "taskType,contextId,streamId,taskId,modelId) VALUES (1, 2,3,4,5,6,7,8,9,10,11);";
    TestCaseDatabaseUtil::InsertData(db, sqlInsert);
    std::vector<TaskPO> taskPOs;
    Dic::Protocol::TaskTable taskTable;
    const uint64_t expectSize = 1;
    const uint64_t index = 0;
    uint64_t initInt = 1;
    taskTable.Select(TaskColumn::ROW_ID, TaskColumn::TIMESTAMP)
        .Select(TaskColumn::ENDTIME, TaskColumn::DECICED_ID)
        .Select(TaskColumn::CONNECTION_ID, TaskColumn::GLOBAL_TASK_ID, TaskColumn::GLOBAL_PID)
        .Select(TaskColumn::TASK_TYPE, TaskColumn::CONTEXT_ID)
        .Select(TaskColumn::STREAM_ID, TaskColumn::TASK_ID)
        .Select(TaskColumn::MODEL_ID)
        .ExcuteQuery(db, taskPOs);
    EXPECT_EQ(taskPOs.size(), expectSize);
    EXPECT_EQ(taskPOs[index].id, initInt);
    EXPECT_EQ(taskPOs[index].timestamp, initInt++);
    EXPECT_EQ(taskPOs[index].endTime, initInt++);
    EXPECT_EQ(taskPOs[index].deviceId, initInt++);
    EXPECT_EQ(taskPOs[index].connectionId, initInt++);
    EXPECT_EQ(taskPOs[index].globalTaskId, initInt++);
    EXPECT_EQ(taskPOs[index].globalPid, initInt++);
    EXPECT_EQ(taskPOs[index].taskType, initInt++);
    EXPECT_EQ(taskPOs[index].contextId, initInt++);
    EXPECT_EQ(taskPOs[index].streamId, initInt++);
    EXPECT_EQ(taskPOs[index].taskId, initInt++);
    EXPECT_EQ(taskPOs[index].modelId, initInt++);
}
