/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include <string>
#include "CommucationOpTable.h"
#include "TestCaseDatabaseUtil.h"
using namespace Dic::Protocol;
using namespace Dic::TimeLine::TestCaseUtil;
class CommucationOpTableTest : public ::testing::Test {};

TEST_F(CommucationOpTableTest, TestCommucationOpTableColumnMaping)
{
    sqlite3 *db = nullptr;
    std::string sql = "CREATE TABLE COMMUNICATION_OP (opName INTEGER,startNs INTEGER,endNs INTEGER,connectionId "
        "INTEGER,groupName INTEGER,opId INTEGER PRIMARY KEY,relay INTEGER,retry INTEGER,dataType "
        "INTEGER,algType INTEGER,count NUMERIC,opType INTEGER, waitNs INTEGER);";
    TestCaseDatabaseUtil::CreateDatabse(db, sql);
    std::string sqlInsert = "INSERT INTO COMMUNICATION_OP (opName, startNs, endNs, connectionId, groupName, opId, "
        "relay,retry,dataType,algType,count,opType,waitNs) VALUES (1, 2,3,4,5,6,7,8,9,10,11,12,13);";
    TestCaseDatabaseUtil::InsertData(db, sqlInsert);
    std::vector<CommucationTaskOpPO> commucationTaskOpPOs;
    Dic::Protocol::CommucationOpTable commucationOpTable;
    const uint64_t expectSize = 1;
    const uint64_t index = 0;
    uint64_t initInt = 1;
    commucationOpTable.Select(CommucationTaskOpColumn::OP_NAME, CommucationTaskOpColumn::TIMESTAMP)
        .Select(CommucationTaskOpColumn::ENDTIME, CommucationTaskOpColumn::CONNECTION_ID)
        .Select(CommucationTaskOpColumn::GROUPNAME, CommucationTaskOpColumn::OP_ID, CommucationTaskOpColumn::RELAY)
        .Select(CommucationTaskOpColumn::RETRY, CommucationTaskOpColumn::DATA_TYPE)
        .Select(CommucationTaskOpColumn::ALG_TYPE, CommucationTaskOpColumn::COUNT)
        .Select(CommucationTaskOpColumn::OP_TYPE, CommucationTaskOpColumn::WAIT_TIME)
        .ExcuteQuery(db, commucationTaskOpPOs);
    EXPECT_EQ(commucationTaskOpPOs.size(), expectSize);
    EXPECT_EQ(commucationTaskOpPOs[index].opName, initInt++);
    EXPECT_EQ(commucationTaskOpPOs[index].timestamp, initInt++);
    EXPECT_EQ(commucationTaskOpPOs[index].endTime, initInt++);
    EXPECT_EQ(commucationTaskOpPOs[index].connectionId, initInt++);
    EXPECT_EQ(commucationTaskOpPOs[index].groupName, initInt++);
    EXPECT_EQ(commucationTaskOpPOs[index].opId, initInt++);
    EXPECT_EQ(commucationTaskOpPOs[index].relay, initInt++);
    EXPECT_EQ(commucationTaskOpPOs[index].retry, initInt++);
    EXPECT_EQ(commucationTaskOpPOs[index].dataType, initInt++);
    EXPECT_EQ(commucationTaskOpPOs[index].algType, initInt++);
    EXPECT_EQ(commucationTaskOpPOs[index].count, initInt++);
    EXPECT_EQ(commucationTaskOpPOs[index].opType, initInt++);
    EXPECT_EQ(commucationTaskOpPOs[index].waitTime, initInt++);
}
