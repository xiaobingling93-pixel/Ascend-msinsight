/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include <string>
#include "../../../DatabaseTestCaseMockUtil.cpp"
#include "OpMemoryTable.h"
using namespace Dic::Module::Memory;
class OpMemoryTableTest : public ::testing::Test {};

TEST_F(OpMemoryTableTest, TestOpMemoryTableColumnMaping)
{
    sqlite3 *db = nullptr;
    Dic::Global::PROFILER::MockUtil::DatabaseTestCaseMockUtil::OpenDB(db);
    std::string sql =
        "CREATE TABLE OP_MEMORY (name INTEGER, size INTEGER, allocationTime INTEGER, releaseTime INTEGER, "
        "activeReleaseTime INTEGER, duration INTEGER, activeDuration INTEGER, allocationTotalAllocated INTEGER, "
        "allocationTotalReserved INTEGER, allocationTotalActive INTEGER, releaseTotalAllocated INTEGER, "
        "releaseTotalReserved INTEGER, releaseTotalActive INTEGER, streamPtr INTEGER, deviceId INTEGER);";
    Dic::Global::PROFILER::MockUtil::DatabaseTestCaseMockUtil::CreateTable(db, sql);
    std::string sqlInsert =
        "INSERT INTO \"main\".\"OP_MEMORY\" (\"name\", \"size\", \"allocationTime\", \"releaseTime\", "
        "\"activeReleaseTime\", \"duration\", \"activeDuration\", \"allocationTotalAllocated\", "
        "\"allocationTotalReserved\", \"allocationTotalActive\", \"releaseTotalAllocated\", \"releaseTotalReserved\", "
        "\"releaseTotalActive\", \"streamPtr\", \"deviceId\") VALUES (2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, "
        "16);";
    Dic::Global::PROFILER::MockUtil::DatabaseTestCaseMockUtil::InsertData(db, sqlInsert);
    std::vector<OpMemoryPO> opMemoryPOs;
    OpMemoryTable operatorTable;
    const uint64_t expectSize = 1;
    const uint64_t index = 0;
    uint64_t initInt = 1;
    operatorTable.Select(OpMemoryColumn::ID, OpMemoryColumn::NAME)
        .Select(OpMemoryColumn::SIZE, OpMemoryColumn::ALLOCATION_TIME)
        .Select(OpMemoryColumn::RELEASE_TIME, OpMemoryColumn::DURATION)
        .Select(OpMemoryColumn::ACTIVE_RELEASE_TIME, OpMemoryColumn::ACTIVE_DURATION)
        .Select(OpMemoryColumn::ALLOCATION_ALLOCATED, OpMemoryColumn::ALLOCATION_RESERVE)
        .Select(OpMemoryColumn::ALLOCATION_ACTIVE, OpMemoryColumn::RELEASE_ALLOCATED)
        .Select(OpMemoryColumn::RELEASE_RESERVE, OpMemoryColumn::RELEASE_ACTIVE)
        .Select(OpMemoryColumn::STREAM, OpMemoryColumn::DEVICE_ID)
        .ExcuteQuery(db, opMemoryPOs);
    EXPECT_EQ(opMemoryPOs.size(), expectSize);
    EXPECT_EQ(opMemoryPOs[index].id, initInt++);
    EXPECT_EQ(opMemoryPOs[index].name, initInt++);
    EXPECT_EQ(opMemoryPOs[index].size, initInt++);
    EXPECT_EQ(opMemoryPOs[index].allocationTime, initInt++);
    EXPECT_EQ(opMemoryPOs[index].releaseTime, initInt++);
    EXPECT_EQ(opMemoryPOs[index].activeReleaseTime, initInt++);
    EXPECT_EQ(opMemoryPOs[index].duration, initInt++);
    EXPECT_EQ(opMemoryPOs[index].activeDuration, initInt++);
    EXPECT_EQ(opMemoryPOs[index].allocationAllocated, initInt++);
    EXPECT_EQ(opMemoryPOs[index].allocationReserve, initInt++);
    EXPECT_EQ(opMemoryPOs[index].allocationActive, initInt++);
    EXPECT_EQ(opMemoryPOs[index].releaseAllocated, initInt++);
    EXPECT_EQ(opMemoryPOs[index].releaseReserve, initInt++);
    EXPECT_EQ(opMemoryPOs[index].releaseActive, initInt++);
    EXPECT_EQ(opMemoryPOs[index].stream, initInt++);
    EXPECT_EQ(opMemoryPOs[index].deviceId, initInt++);
}
