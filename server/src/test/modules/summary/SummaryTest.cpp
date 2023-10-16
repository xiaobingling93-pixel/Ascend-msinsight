/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "SummaryProtocolRequest.h"
#include "DataBaseManager.h"
#include "../../TestSuit.cpp"

class SummaryTest : TestSuit {
};

TEST_F(TestSuit, QueryComputeStatisticsData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::SummaryStatisticParams requestParams;
    requestParams.rankId = "0";
    Dic::Protocol::SummaryStatisticsBody responseBody;
    database->QueryComputeStatisticsData(requestParams, responseBody);
    int expectSize = 5;
    EXPECT_EQ(responseBody.summaryStatisticsItemList.size(), expectSize);
}

TEST_F(TestSuit, QueryComputeStatisticsData2)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::SummaryStatisticParams requestParams;
    requestParams.rankId = "0";
    requestParams.stepId = "2";
    Dic::Protocol::SummaryStatisticsBody responseBody;
    database->QueryComputeStatisticsData(requestParams, responseBody);
    int expectSize = 5;
    EXPECT_EQ(responseBody.summaryStatisticsItemList.size(), expectSize);
}