/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include <string>
#include "CannApiTable.h"
#include "TestCaseDatabaseUtil.h"
using namespace Dic::Protocol;
using namespace Dic::TimeLine::TestCaseUtil;
class CannApiTableTest : public ::testing::Test {};

TEST_F(CannApiTableTest, TestCannApiTableColumnMaping)
{
    sqlite3 *db = nullptr;
    std::string sql = "CREATE TABLE  CANN_API ("
        "  startNs INTEGER,"
        "  endNs INTEGER,"
        "  type INTEGER,"
        "  globalTid INTEGER,"
        "  connectionId INTEGER,"
        "  name INTEGER,"
        "  depth INTEGER,"
        " PRIMARY KEY (connectionId)"
        ");";
    TestCaseDatabaseUtil::CreateDatabse(db, sql);
    std::string sqlInsert =
        "INSERT INTO CANN_API (startNs, endNs, type, globalTid, connectionId, name, depth) VALUES (1, 2,3,4,5,6,7);";
    TestCaseDatabaseUtil::InsertData(db, sqlInsert);
    std::vector<CannApiPO> cannApiPOs;
    Dic::Protocol::CannApiTable cannApiTable;
    const uint64_t expectSize = 1;
    const uint64_t index = 0;
    uint64_t initInt = 1;
    cannApiTable.Select(CannApiColumn::ID, CannApiColumn::TIMESTAMP)
        .Select(CannApiColumn::ENDTIME, CannApiColumn::TYPE)
        .Select(CannApiColumn::GLOBAL_TID, CannApiColumn::NAME, CannApiColumn::DEPTH)
        .ExcuteQuery(db, cannApiPOs);
    EXPECT_EQ(cannApiPOs.size(), expectSize);
    EXPECT_EQ(cannApiPOs[index].timestamp, initInt++);
    EXPECT_EQ(cannApiPOs[index].endTime, initInt++);
    EXPECT_EQ(cannApiPOs[index].type, initInt++);
    EXPECT_EQ(cannApiPOs[index].globalTid, initInt++);
    EXPECT_EQ(cannApiPOs[index].id, initInt++);
    EXPECT_EQ(cannApiPOs[index].name, initInt++);
    EXPECT_EQ(cannApiPOs[index].depth, initInt++);
}
