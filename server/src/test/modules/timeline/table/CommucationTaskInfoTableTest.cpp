/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include <string>
#include "CommucationTaskInfoTable.h"
#include "TestCaseDatabaseUtil.h"
using namespace Dic::Protocol;
using namespace Dic::TimeLine::TestCaseUtil;
class CommucationTaskInfoTableTest : public ::testing::Test {};

TEST_F(CommucationTaskInfoTableTest, TestCommucationTaskInfoTableColumnMaping)
{
    sqlite3 *db = nullptr;
    std::string sql =
        "CREATE TABLE COMMUNICATION_TASK_INFO (name INTEGER,globalTaskId INTEGER,taskType INTEGER,planeId "
        "INTEGER,groupName INTEGER,notifyId INTEGER,rdmaType INTEGER,srcRank INTEGER,dstRank INTEGER,transportType "
        "INTEGER,size INTEGER,dataType INTEGER,linkType INTEGER,opId INTEGER);";
    TestCaseDatabaseUtil::CreateDatabse(db, sql);
    std::string sqlInsert = "INSERT INTO COMMUNICATION_TASK_INFO (name, globalTaskId, taskType, planeId, groupName, "
        "notifyId,rdmaType,srcRank,dstRank,transportType,size,dataType,linkType,opId) VALUES (1, "
        "2,3,4,5,6,7,8,9,10,11,12,13,14);";
    TestCaseDatabaseUtil::InsertData(db, sqlInsert);
    std::vector<CommucationTaskInfoPO> commucationTaskInfoPOs;
    Dic::Protocol::CommucationTaskInfoTable commucationTaskInfoTable;
    const uint64_t expectSize = 1;
    const uint64_t index = 0;
    uint64_t initInt = 1;
    commucationTaskInfoTable.Select(CommucationTaskInfoColumn::ROW_ID, CommucationTaskInfoColumn::NAME)
        .Select(CommucationTaskInfoColumn::GLOBAL_TASK_ID, CommucationTaskInfoColumn::TASK_TYPE)
        .Select(CommucationTaskInfoColumn::PLANE_ID, CommucationTaskInfoColumn::GROUPNAME,
        CommucationTaskInfoColumn::NOTIFY_ID)
        .Select(CommucationTaskInfoColumn::RDMA_TYPE, CommucationTaskInfoColumn::SRC_RANK)
        .Select(CommucationTaskInfoColumn::DST_RANK, CommucationTaskInfoColumn::TRANSPORT_TYPE)
        .Select(CommucationTaskInfoColumn::SIZE, CommucationTaskInfoColumn::DATA_TYPE)
        .Select(CommucationTaskInfoColumn::OP_ID, CommucationTaskInfoColumn::LINK_TYPE)
        .ExcuteQuery(db, commucationTaskInfoPOs);
    EXPECT_EQ(commucationTaskInfoPOs.size(), expectSize);
    EXPECT_EQ(commucationTaskInfoPOs[index].id, initInt);
    EXPECT_EQ(commucationTaskInfoPOs[index].name, initInt++);
    EXPECT_EQ(commucationTaskInfoPOs[index].globalTaskId, initInt++);
    EXPECT_EQ(commucationTaskInfoPOs[index].taskType, initInt++);
    EXPECT_EQ(commucationTaskInfoPOs[index].planeId, initInt++);
    EXPECT_EQ(commucationTaskInfoPOs[index].groupName, initInt++);
    EXPECT_EQ(commucationTaskInfoPOs[index].notifyId, initInt++);
    EXPECT_EQ(commucationTaskInfoPOs[index].rdmaType, initInt++);
    EXPECT_EQ(commucationTaskInfoPOs[index].srcRank, initInt++);
    EXPECT_EQ(commucationTaskInfoPOs[index].dstRank, initInt++);
    EXPECT_EQ(commucationTaskInfoPOs[index].transportType, initInt++);
    EXPECT_EQ(commucationTaskInfoPOs[index].size, initInt++);
    EXPECT_EQ(commucationTaskInfoPOs[index].dataType, initInt++);
    EXPECT_EQ(commucationTaskInfoPOs[index].linkType, initInt++);
    EXPECT_EQ(commucationTaskInfoPOs[index].opId, initInt++);
}
