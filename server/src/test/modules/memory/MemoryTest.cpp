/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MemoryProtocolRequest.h"
#include "DataBaseManager.h"
#include "../../TestSuit.cpp"

class MemoryTest : TestSuit {
};

TEST_F(TestSuit, QueryMemoryOperatorData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.currentPage = 0;
    requestParams.pageSize = 10; // page size = 10
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    database->QueryOperatorDetail(requestParams, responseBody);
    int expectSize = 10;
    EXPECT_EQ(responseBody.size(), expectSize);
}

TEST_F(TestSuit, QueryMemoryViewData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "0";
    Dic::Protocol::OperatorMemory responseBody;
    database->QueryMemoryView(requestParams, responseBody);
    int expectSize = 5;
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(TestSuit, QueryOperatorSizeData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    double min;
    double max;
    database->QueryOperatorSize(min, max);
    int expectMin = 64;
    int expectMax = 81984;
    EXPECT_EQ(min, expectMin);
    EXPECT_EQ(max, expectMax);
}
