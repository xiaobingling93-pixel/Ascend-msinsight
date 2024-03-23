/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "../../FullDbTestSuit.cpp"

class DbTimelineTestSuit : FullDbTestSuit {
};

TEST_F(FullDbTestSuit, FullDb_of_SearchSliceNameCount)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("FullDb");
    int expectCount = 9;
    Dic::Protocol::SearchCountParams params;
    params.searchContent = "hcom";
    params.rankId = "2"; // cardId = 2

    auto count = database->SearchSliceNameCount(params);
    EXPECT_EQ(count, expectCount);
}

TEST_F(FullDbTestSuit, FullDb_of_SearchSliceName)
{
    const uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("FullDb");
    std::string sliceName = "hcom";
    int index = 0;
    Dic::Protocol::SearchSliceBody body;

    body.rankId = "2"; // cardId = 2
    std::string expectPid = "ASCEND HARDWARE";
    std::string expectTid = "8";
    uint64_t expectStartTime = 181306181;
    int32_t expectDepth = 0;
    uint64_t expectDuration = 51121;

    database->SearchSliceName(sliceName, index, minTimestamp, body);
    EXPECT_EQ(body.pid, expectPid);
    EXPECT_EQ(body.tid, expectTid);
    EXPECT_EQ(body.startTime, expectStartTime);
    EXPECT_EQ(body.depth, expectDepth);
    EXPECT_EQ(body.duration, expectDuration);
}

TEST_F(FullDbTestSuit, FullDb_of_ThreadTraces)
{
    const uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("FullDb");
    Dic::Protocol::UnitThreadTracesBody body;

    Dic::Protocol::UnitThreadTracesParams params;
    params.startTime = 0;
    params.cardId = "2";
    params.processId = "ASCEND HARDWARE";
    params.metaType = "ASCEND HARDWARE";
    params.threadId = "8";
    params.endTime = 400000000; // endTime = 400000000
    params.timePerPx = 1;

    database->QueryThreadTraces(params, body, minTimestamp, 0);
    EXPECT_EQ(body.data.size(), 1);
    EXPECT_EQ(body.data[0].size(), 1);
    EXPECT_EQ(body.data[0][0].startTime, 181306181); // startTime = 181306181
    EXPECT_EQ(body.data[0][0].endTime, 181357302); // endTime = 181357302
    EXPECT_EQ(body.data[0][0].id, "49210");
    EXPECT_EQ(body.data[0][0].name, "hcom_allReduce__305_880_1");

    body.data.clear();
    params.metaType = "HCCL";
    params.threadId = "1";

    database->QueryThreadTraces(params, body, minTimestamp, 0);
    EXPECT_EQ(body.data.size(), 1);
    EXPECT_EQ(body.data[0].size(), 1);
    EXPECT_EQ(body.data[0][0].startTime, 175902293); // startTime = 175902293
    EXPECT_EQ(body.data[0][0].endTime, 175903813); // endTime = 175903813
    EXPECT_EQ(body.data[0][0].id, "46919");
    EXPECT_EQ(body.data[0][0].name, "hcom_allGather__305_879_1");

    body.data.clear();
    params.metaType = "HOST";
    params.threadId = "20000";
    params.processId = "1181473";

    database->QueryThreadTraces(params, body, minTimestamp, 0);
    EXPECT_EQ(body.data.size(), 1);
    EXPECT_EQ(body.data[0].size(), 3); // size = 3
    EXPECT_EQ(body.data[0][0].startTime, 39360870); // startTime = 39360870
    EXPECT_EQ(body.data[0][0].endTime, 39363430); // endTime = 39363430
    EXPECT_EQ(body.data[0][0].id, "0");
    EXPECT_EQ(body.data[0][0].name, "aclrtGetDeviceCount");

    params.metaType = "HBM";
    EXPECT_EQ(database->QueryThreadTraces(params, body, minTimestamp, 0), false);
}