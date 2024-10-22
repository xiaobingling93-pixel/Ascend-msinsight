/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include <string>
#include "HostInfoTable.h"
#include "TestCaseDatabaseUtil.h"
using namespace Dic::Protocol;
using namespace Dic::TimeLine::TestCaseUtil;
class HostInfoTableTest : public ::testing::Test {};

TEST_F(HostInfoTableTest, TestHostInfoTableColumnMaping)
{
    sqlite3 *db = nullptr;
    std::string sql = "CREATE TABLE  HOST_INFO ("
        "  hostUid TEXT,"
        "  hostName TEXT"
        ");";
    TestCaseDatabaseUtil::CreateDatabse(db, sql);
    std::string sqlInsert = "INSERT INTO HOST_INFO (hostUid, hostName) VALUES ('yyyy', 'djsjjksd'), ('zzzz', 'wwww');";
    TestCaseDatabaseUtil::InsertData(db, sqlInsert);
    std::vector<HostInfoPO> hostInfoPOs;
    HostInfoTable hostInfoTable;
    const uint64_t expectSize = 2;
    const std::string expectHostUid = "zzzz";
    const std::string expectHostName = "wwww";
    const uint64_t index = 1;
    hostInfoTable.Select(HostInfoColumn::HOST_UID, HostInfoColumn::HOST_NAME).ExcuteQuery(db, hostInfoPOs);
    EXPECT_EQ(hostInfoPOs.size(), expectSize);
    EXPECT_EQ(hostInfoPOs[index].hostUid, expectHostUid);
    EXPECT_EQ(hostInfoPOs[index].hostName, expectHostName);
}
