/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include <string>
#include "FlowTable.h"
#include "TestCaseDatabaseUtil.h"
using namespace Dic::Protocol;
using namespace Dic::TimeLine::TestCaseUtil;
class FlowTableTest : public ::testing::Test {};

TEST_F(FlowTableTest, TestFlowTableColumnMaping)
{
    sqlite3 *db = nullptr;
    std::string sql = "CREATE TABLE flow (id INTEGER PRIMARY KEY AUTOINCREMENT, flow_id TEXT, name TEXT, cat TEXT, "
        "track_id INTEGER, timestamp INTEGER, type TEXT);";
    TestCaseDatabaseUtil::CreateDatabse(db, sql);
    std::string sqlInsert =
        "INSERT INTO flow (flow_id, name, cat, track_id, timestamp, type) VALUES ('1', '2','3',5,6,'7');";
    TestCaseDatabaseUtil::InsertData(db, sqlInsert);
    std::vector<FlowPO> flowPOs;
    Dic::Protocol::FlowTable flowTable;
    const uint64_t expectSize = 1;
    const uint64_t expectId = 1;
    const uint64_t index = 0;
    const std::string expectFlowId = "1";
    const std::string expectName = "2";
    const std::string expectCat = "3";
    const std::string expectType = "7";
    const uint64_t expectTrackId = 5;
    const uint64_t expectTime = 6;
    flowTable.Select(FlowColumn::ID, FlowColumn::FLOW_ID)
        .Select(FlowColumn::NAME, FlowColumn::CAT)
        .Select(FlowColumn::TRACK_ID, FlowColumn::TIMESTAMP, FlowColumn::TYPE)
        .ExcuteQuery(db, flowPOs);
    EXPECT_EQ(flowPOs.size(), expectSize);
    EXPECT_EQ(flowPOs[index].id, expectId);
    EXPECT_EQ(flowPOs[index].flowId, expectFlowId);
    EXPECT_EQ(flowPOs[index].name, expectName);
    EXPECT_EQ(flowPOs[index].cat, expectCat);
    EXPECT_EQ(flowPOs[index].timestamp, expectTime);
    EXPECT_EQ(flowPOs[index].trackId, expectTrackId);
    EXPECT_EQ(flowPOs[index].type, expectType);
}
