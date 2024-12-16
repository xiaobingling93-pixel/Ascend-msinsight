/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include <string>
#include "../../../DatabaseTestCaseMockUtil.cpp"
#include "OperatorTable.h"
using namespace Dic::Module::Memory;
class OperatorTableTest : public ::testing::Test {};

TEST_F(OperatorTableTest, TestOperatorTableColumnMaping)
{
    sqlite3 *db = nullptr;
    Dic::Global::PROFILER::MockUtil::DatabaseTestCaseMockUtil::OpenDB(db);
    std::string sql =
        "CREATE TABLE operator (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, size INTEGER, allocation_time "
        "INTEGER, release_time INTEGER, duration INTEGER, active_release_time INTEGER, active_duration INTEGER, "
        "allocation_allocated INTEGER, allocation_reserve INTEGER, allocation_active INTEGER, release_allocated "
        "INTEGER, release_reserve INTEGER, release_active INTEGER, stream TEXT);";
    Dic::Global::PROFILER::MockUtil::DatabaseTestCaseMockUtil::CreateTable(db, sql);
    std::string sqlInsert =
        "INSERT INTO \"main\".\"operator\" (\"id\", \"name\", \"size\", \"allocation_time\", \"release_time\", "
        "\"duration\", \"active_release_time\", \"active_duration\", \"allocation_allocated\", \"allocation_reserve\", "
        "\"allocation_active\", \"release_allocated\", \"release_reserve\", \"release_active\", \"stream\") VALUES (1, "
        "'aten::empty_strided', 2, 3, 4, 5, 6, "
        "7, 8, 9, 10, 11, 12, 13, "
        "'187651271017536');";
    Dic::Global::PROFILER::MockUtil::DatabaseTestCaseMockUtil::InsertData(db, sqlInsert);
    std::vector<OperatorPO> operatorPos;
    OperatorTable operatorTable;
    const uint64_t expectSize = 1;
    const uint64_t index = 0;
    uint64_t initInt = 1;
    operatorTable.Select(OperatorColumn::ID, OperatorColumn::NAME)
        .Select(OperatorColumn::SIZE, OperatorColumn::ALLOCATION_TIME)
        .Select(OperatorColumn::RELEASE_TIME, OperatorColumn::DURATION)
        .Select(OperatorColumn::ACTIVE_RELEASE_TIME, OperatorColumn::ACTIVE_DURATION)
        .Select(OperatorColumn::ALLOCATION_ALLOCATED, OperatorColumn::ALLOCATION_RESERVE)
        .Select(OperatorColumn::ALLOCATION_ACTIVE, OperatorColumn::RELEASE_ALLOCATED)
        .Select(OperatorColumn::RELEASE_RESERVE, OperatorColumn::RELEASE_ACTIVE, OperatorColumn::STREAM)
        .ExcuteQuery(db, operatorPos);
    EXPECT_EQ(operatorPos.size(), expectSize);
    EXPECT_EQ(operatorPos[index].id, initInt++);
    EXPECT_EQ(operatorPos[index].name, "aten::empty_strided");
    EXPECT_EQ(operatorPos[index].size, initInt++);
    EXPECT_EQ(operatorPos[index].allocationTime, initInt++);
    EXPECT_EQ(operatorPos[index].releaseTime, initInt++);
    EXPECT_EQ(operatorPos[index].duration, initInt++);
    EXPECT_EQ(operatorPos[index].activeReleaseTime, initInt++);
    EXPECT_EQ(operatorPos[index].activeDuration, initInt++);
    EXPECT_EQ(operatorPos[index].allocationAllocated, initInt++);
    EXPECT_EQ(operatorPos[index].allocationReserve, initInt++);
    EXPECT_EQ(operatorPos[index].allocationActive, initInt++);
    EXPECT_EQ(operatorPos[index].releaseAllocated, initInt++);
    EXPECT_EQ(operatorPos[index].releaseReserve, initInt++);
    EXPECT_EQ(operatorPos[index].releaseActive, initInt++);
    EXPECT_EQ(operatorPos[index].stream, "187651271017536");
}
