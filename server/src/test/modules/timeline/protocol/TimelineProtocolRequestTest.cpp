/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "TimelineProtocolRequest.h"
class TimelineProtocolRequestTest : public ::testing::Test {};

TEST_F(TimelineProtocolRequestTest, TestImportActionParams)
{
    Dic::Protocol::ImportActionParams params;
    params.projectName = "ll";
    params.projectAction = Dic::Protocol::ProjectActionEnum::UNKNOWN;
    std::string errorMsg;
    bool res = params.CommonCheck(errorMsg);
    EXPECT_EQ(res, false);
    params.projectAction = Dic::Protocol::ProjectActionEnum::TRANSFER_PROJECT;
    res = params.CommonCheck(errorMsg);
    EXPECT_EQ(res, true);
    res = params.ConvertToRealPath(errorMsg);
    EXPECT_EQ(res, false);
    params.path.emplace_back("LLLLLLLLLL");
    res = params.ConvertToRealPath(errorMsg);
    EXPECT_EQ(res, false);
}

TEST_F(TimelineProtocolRequestTest, TestUnitThreadTracesParams)
{
    Dic::Protocol::UnitThreadTracesParams params;
    const uint64_t st = 9;
    const uint64_t en = 2;
    params.startTime = st;
    params.endTime = en;
    std::string errorMsg;
    const uint64_t min = 7;
    bool res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
    params.endTime = UINT64_MAX;
    res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
    const uint64_t mi = 89;
    params.endTime = mi;
    res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, true);
}

TEST_F(TimelineProtocolRequestTest, UnitThreadTracesSummaryParams)
{
    Dic::Protocol::UnitThreadTracesSummaryParams params;
    const uint64_t st = 9;
    const uint64_t en = 2;
    params.startTime = st;
    params.endTime = en;
    std::string errorMsg;
    const uint64_t min = 7;
    bool res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
    params.endTime = UINT64_MAX;
    res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
    const uint64_t mi = 89;
    params.endTime = mi;
    res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, true);
}

TEST_F(TimelineProtocolRequestTest, UnitThreadsParams)
{
    Dic::Protocol::UnitThreadsParams params;
    const uint64_t st = 9;
    const uint64_t en = 2;
    params.startTime = st;
    params.endTime = en;
    std::string errorMsg;
    const uint64_t min = 7;
    bool res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
    params.endTime = UINT64_MAX;
    res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
    const uint64_t mi = 89;
    std::string startDepth = "";
    std::string endDepth = "";
    params.endTime = mi;
    params.startDepth = startDepth;
    params.endDepth = endDepth;
    res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, true);
}

TEST_F(TimelineProtocolRequestTest, UnitFlowsParams)
{
    Dic::Protocol::UnitFlowsParams params;
    const uint64_t st = 9;
    const uint64_t en = 2;
    params.startTime = st;
    params.endTime = en;
    std::string errorMsg;
    const uint64_t min = 7;
    bool res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
    params.endTime = UINT64_MAX;
    res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
    const uint64_t mi = 89;
    params.endTime = mi;
    res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, true);
}

TEST_F(TimelineProtocolRequestTest, FlowCategoryEventsParams)
{
    Dic::Protocol::FlowCategoryEventsParams params;
    const uint64_t st = 9;
    const uint64_t en = 2;
    params.startTime = st;
    params.endTime = en;
    std::string errorMsg;
    const uint64_t min = 7;
    bool res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
    params.endTime = UINT64_MAX;
    res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
    const uint64_t mi = 89;
    params.endTime = mi;
    res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, true);
}

TEST_F(TimelineProtocolRequestTest, TestUnitCounterParams)
{
    Dic::Protocol::UnitCounterParams params;
    const uint64_t st = 9;
    const uint64_t en = 2;
    params.startTime = st;
    params.endTime = en;
    std::string errorMsg;
    const uint64_t min = 7;
    bool res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
    params.endTime = UINT64_MAX;
    res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
    const uint64_t mi = 89;
    params.endTime = mi;
    res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, true);
}

TEST_F(TimelineProtocolRequestTest, TestUnitThreadsOperatorsParams)
{
    Dic::Protocol::UnitThreadsOperatorsParams params;
    const uint64_t st = 9;
    const uint64_t en = 2;
    params.startTime = st;
    params.endTime = en;
    std::string errorMsg;
    const uint64_t min = 7;
    bool res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
    params.endTime = UINT64_MAX;
    res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
    const uint64_t mi = 89;
    params.endTime = mi;
    res = params.CheckParams(min, errorMsg);
    EXPECT_EQ(res, false);
}