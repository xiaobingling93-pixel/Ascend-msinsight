/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "TimelineProtocolRequest.h"
#include "DataBaseManager.h"
#include "../../TestSuit.cpp"

class TimelineTest : TestSuit {
};

TEST_F(TestSuit, QueryPythonViewData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::SystemViewParams requestParams;
    Dic::Protocol::SystemViewBody responseBody;
    uint64_t PAGE = 10;
    requestParams.layer = "Python";
    requestParams.current = 1;
    requestParams.order = "descend";
    requestParams.orderBy = "name";
    requestParams.pageSize = PAGE;
    database->QueryPythonViewData(requestParams, responseBody);
    int expectSize = 10;
    EXPECT_EQ(responseBody.systemViewDetail.size(), expectSize);
}

TEST_F(TestSuit, QueryPythonViewWithTotalNum)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::SystemViewParams requestParams;
    Dic::Protocol::SystemViewBody responseBody;
    uint64_t PAGE = 10;
    requestParams.layer = "Python";
    requestParams.current = 1;
    requestParams.order = "descend";
    requestParams.orderBy = "name";
    requestParams.pageSize = PAGE;
    requestParams.isQueryTotal = true;
    database->QueryPythonViewData(requestParams, responseBody);
    int expectSize = 162;
    EXPECT_EQ(responseBody.total, expectSize);
}

TEST_F(TestSuit, QueryLayerOperatorTimeWithCann)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");

    double layerOperatorTime = database->QueryLayerOperatorTime("CANN");
    double expectSize = 158266100;
    EXPECT_EQ(layerOperatorTime, expectSize);
}

TEST_F(TestSuit, QueryLayerOperatorTimeWithPython)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");

    double layerOperatorTime = database->QueryLayerOperatorTime("Python");
    double expectSize = 851869940;
    EXPECT_EQ(layerOperatorTime, expectSize);
}

TEST_F(TestSuit, QueryLayerOperatorTimeWithAscend)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");

    double layerOperatorTime = database->QueryLayerOperatorTime("Ascend Hardware");
    double expectSize = 842011262;
    EXPECT_EQ(layerOperatorTime, expectSize);
}

TEST_F(TestSuit, QueryLayerOperatorTimeWithHCCL)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");

    double layerOperatorTime = database->QueryLayerOperatorTime("HCCL");
    double expectSize = 449202040;
    EXPECT_EQ(layerOperatorTime, expectSize);
}

TEST_F(TestSuit, QueryLayerOperatorTimeWithOverlap)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");

    double layerOperatorTime = database->QueryLayerOperatorTime("Overlap Analysis");
    double expectSize = 445796394;
    EXPECT_EQ(layerOperatorTime, expectSize);
}

TEST_F(TestSuit, QueryCoreType)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");

    const std::vector<std::string> &coreType = database->QueryCoreType();
    double expectSize = 7;
    EXPECT_EQ(coreType.size(), expectSize);
}

TEST_F(TestSuit, QueryKernelDetailData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::KernelDetailsParams requestParams;
    Dic::Protocol::KernelDetailsBody responseBody;
    uint64_t PAGE = 10;
    requestParams.current = 1;
    requestParams.order = "descend";
    requestParams.orderBy = "name";
    requestParams.pageSize = PAGE;
    requestParams.rankId = "0";
    database->QueryKernelDetailData(requestParams, responseBody, 0);
    int expectSize = 21;
    EXPECT_EQ(responseBody.count, expectSize);
}

TEST_F(TestSuit, QueryKernelDetailDataWithCoreType)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::KernelDetailsParams requestParams;
    Dic::Protocol::KernelDetailsBody responseBody;
    uint64_t PAGE = 10;
    requestParams.current = 1;
    requestParams.order = "descend";
    requestParams.orderBy = "name";
    requestParams.pageSize = PAGE;
    requestParams.rankId = "0";
    requestParams.coreType = "AI_CORE";
    database->QueryKernelDetailData(requestParams, responseBody, 0);
    int expectSize = 4;
    EXPECT_EQ(responseBody.count, expectSize);
}

TEST_F(TestSuit, QueryKernelDepthAndThread)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetTraceDatabase("0");
    Dic::Protocol::KernelParams requestParams;
    Dic::Protocol::OneKernelBody responseBody;
    uint64_t DURATION = 72861;
    requestParams.duration = DURATION;
    requestParams.timestamp = "1695115378736217088";
    requestParams.name = "trans_Cast_15";
    database->QueryKernelDepthAndThread(requestParams, responseBody);
    uint64_t depth = 0;
    uint64_t tid = 17;
    EXPECT_EQ(responseBody.depth, depth);
    EXPECT_EQ(responseBody.threadId, tid);
}
