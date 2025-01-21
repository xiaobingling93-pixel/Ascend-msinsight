/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include <gtest/gtest.h>
#include "CommunicationProtocolResponse.h"

using namespace Dic::Protocol;
class CommunicationProtocolResponseTest : public ::testing::Test {};

/**
 * 正常采集八卡的情况
 */
TEST_F(CommunicationProtocolResponseTest, TestAdjustTimeWhen8RankNormal)
{
    OperatorListsResponseBody body;
    const int rankSize = 8;
    const uint64_t min = 1;
    const uint64_t max = 11;
    uint64_t init = min;
    for (int i = 0; i < rankSize; ++i) {
        CompareData<std::vector<OperatorTimeItem>> data1;
        OperatorTimeItem operatorTime1 = { "", init++, max };
        body.maxTime = std::max(body.maxTime, operatorTime1.startTime + operatorTime1.elapseTime);
        body.minTime = std::min(body.minTime, operatorTime1.startTime);
        data1.compare.emplace_back(operatorTime1);
        body.opLists.emplace_back(data1);
    }
    body.AdjustTime("");
    const uint64_t expectMax = 19;
    EXPECT_EQ(body.maxTime, expectMax);
    EXPECT_EQ(body.minTime, min);
    EXPECT_EQ(body.opLists.size(), rankSize);
    EXPECT_EQ(body.opLists.front().compare.front().startTime, min);
    EXPECT_EQ(body.opLists.back().compare.back().elapseTime, max);
}

/**
 * 两次采集的数据，每次都是8卡
 */
TEST_F(CommunicationProtocolResponseTest, TestAdjustTimeWhen16Rank2Group)
{
    OperatorListsResponseBody body;
    const int rankSize = 8;
    const uint64_t min = 1;
    const uint64_t max = 11;
    uint64_t init = min;
    for (int i = 0; i < rankSize; ++i) {
        CompareData<std::vector<OperatorTimeItem>> data1;
        OperatorTimeItem operatorTime1 = { "", init++, max };
        body.maxTime = std::max(body.maxTime, operatorTime1.startTime + operatorTime1.elapseTime);
        body.minTime = std::min(body.minTime, operatorTime1.startTime);
        data1.compare.emplace_back(operatorTime1);
        body.opLists.emplace_back(data1);
    }

    const int secondMin = 100;
    init = secondMin;
    for (int i = 0; i < rankSize; ++i) {
        CompareData<std::vector<OperatorTimeItem>> data1;
        OperatorTimeItem operatorTime1 = { "", init++, max };
        body.maxTime = std::max(body.maxTime, operatorTime1.startTime + operatorTime1.elapseTime);
        body.minTime = std::min(body.minTime, operatorTime1.startTime);
        data1.compare.emplace_back(operatorTime1);
        body.opLists.emplace_back(data1);
    }
    body.AdjustTime("");
    const uint64_t expectMax = 118;
    EXPECT_EQ(body.maxTime, expectMax);
    const uint64_t expectMin = 100;
    EXPECT_EQ(body.minTime, expectMin);
    EXPECT_EQ(body.opLists.size(), rankSize + rankSize);
    EXPECT_EQ(body.opLists.front().compare.front().startTime, expectMin);
    EXPECT_EQ(body.opLists.back().compare.back().elapseTime, max);
}

/**
 * 采集八卡,对比情况,base和baseline在同一时间范围内
 */
TEST_F(CommunicationProtocolResponseTest, TestAdjustTimeWhen8RankAndHaveBaseWithOneGroup)
{
    OperatorListsResponseBody body;
    const int rankSize = 8;
    const uint64_t min = 1;
    const uint64_t max = 11;
    uint64_t init = min;
    for (int i = 0; i < rankSize; ++i) {
        CompareData<std::vector<OperatorTimeItem>> data1;
        OperatorTimeItem operatorTime1 = { "", init++, max };
        body.maxTime = std::max(body.maxTime, operatorTime1.startTime + operatorTime1.elapseTime);
        body.minTime = std::min(body.minTime, operatorTime1.startTime);
        data1.compare.emplace_back(operatorTime1);
        data1.baseline.emplace_back(operatorTime1);
        body.opLists.emplace_back(data1);
    }
    body.AdjustTime("");
    const uint64_t expectMax = 19;
    EXPECT_EQ(body.maxTime, expectMax);
    EXPECT_EQ(body.minTime, min);
    EXPECT_EQ(body.opLists.size(), rankSize);
    EXPECT_EQ(body.opLists.front().compare.front().startTime, min);
    EXPECT_EQ(body.opLists.back().compare.back().elapseTime, max);
    EXPECT_EQ(body.opLists.front().baseline.front().startTime, min);
    EXPECT_EQ(body.opLists.back().baseline.back().elapseTime, max);
}

/**
 * 采集八卡,对比情况,base和baseline不在同一时间范围内
 */
TEST_F(CommunicationProtocolResponseTest, TestAdjustTimeWhen8RankAndHaveBaseWithTwoGroup)
{
    OperatorListsResponseBody body;
    const int rankSize = 8;
    const uint64_t min = 1;
    const uint64_t max = 11;
    const uint64_t gap = 10000000000000;
    uint64_t init = min;
    for (int i = 0; i < rankSize; ++i) {
        CompareData<std::vector<OperatorTimeItem>> data1;
        OperatorTimeItem operatorTime1 = { "", init++, max };
        body.maxTime = std::max(body.maxTime, operatorTime1.startTime + operatorTime1.elapseTime);
        body.minTime = std::min(body.minTime, operatorTime1.startTime);
        data1.compare.emplace_back(operatorTime1);
        operatorTime1.startTime += gap;
        body.maxTime = std::max(body.maxTime, operatorTime1.startTime + operatorTime1.elapseTime);
        body.minTime = std::min(body.minTime, operatorTime1.startTime);
        data1.baseline.emplace_back(operatorTime1);
        body.opLists.emplace_back(data1);
    }
    body.AdjustTime("");
    const uint64_t expectMax = 10000000000019;
    EXPECT_EQ(body.maxTime, expectMax);
    const uint64_t expectMin = 10000000000001;
    EXPECT_EQ(body.minTime, expectMin);
    EXPECT_EQ(body.opLists.size(), rankSize);
    EXPECT_EQ(body.opLists.front().compare.front().startTime, expectMin);
    EXPECT_EQ(body.opLists.back().compare.back().elapseTime, max);
    EXPECT_EQ(body.opLists.front().baseline.front().startTime, expectMin);
    EXPECT_EQ(body.opLists.back().baseline.back().elapseTime, max);
}