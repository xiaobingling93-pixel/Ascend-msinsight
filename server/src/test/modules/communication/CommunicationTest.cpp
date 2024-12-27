/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "CommunicationProtocolRequest.h"
#include "DataBaseManager.h"
#include "ClusterDomainObject.h"
#include "../../TestSuit.cpp"

class CommunicationTest : TestSuit {
};

TEST_F(TestSuit, QueryIterationsData)
{
    DataBaseManager::Instance().SetDataType(DataType::TEXT);
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::vector<Dic::Protocol::IterationsOrRanksObject> responseBody;
    database->QueryIterations(responseBody);
    int expectSize = 1;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(responseBody[0].iterationOrRankId, "2");
}

TEST_F(TestSuit, QueryOperatorNameData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::OperatorNamesParams requestParams;
    requestParams.dbIndex = "0";
    requestParams.iterationId = "2";
    requestParams.stage = "p2p";
    std::vector<Dic::Protocol::OperatorNamesObject> responseBody;
    database->QueryOperatorNames(requestParams, responseBody);
    int expectSize = 2;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(responseBody[0].operatorName, "Total Op Info");
    EXPECT_EQ(responseBody[1].operatorName, "hcom_send__822_0");
}

TEST_F(TestSuit, QueryIterationAndCommunicationGroupSuccess)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::CommunicationKernelParams requestParams;
    requestParams.name = "hcom_broadcast__483_0";
    requestParams.rankId = "0";
    Dic::Protocol::CommunicationKernelBody responseBody;
    database->QueryIterationAndCommunicationGroup(requestParams, responseBody);
    EXPECT_EQ(responseBody.step, "2");
    EXPECT_EQ(responseBody.group, "(0, 1, 2, 3, 4, 5, 6, 7)");
}

TEST_F(TestSuit, QueryOperatorNameDataWithRank)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::OperatorNamesParams requestParams;
    requestParams.dbIndex = "0";
    requestParams.iterationId = "2";
    requestParams.stage = "p2p";
    requestParams.rankList = {"0"};
    std::vector<Dic::Protocol::OperatorNamesObject> responseBody;
    database->QueryOperatorNames(requestParams, responseBody);
    int expectSize = 2;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(responseBody[0].operatorName, "Total Op Info");
    EXPECT_EQ(responseBody[1].operatorName, "hcom_send__822_0");
}

TEST_F(TestSuit, QueryRanksData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::RanksParams requestParam;
    requestParam.dbIndex = "0";
    requestParam.iterationId = "2";
    std::vector<Dic::Protocol::IterationsOrRanksObject> responseBody;
    database->QueryRanksHandler(responseBody);
    int expectSize = 16;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(responseBody[0].iterationOrRankId, "0");
    EXPECT_EQ(responseBody[1].iterationOrRankId, "1");
}

TEST_F(TestSuit, QueryDurationData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::DurationListParams requestParams;
    std::vector<Dic::Module::DurationDo> durationList;
    requestParams.dbIndex = "0";
    requestParams.iterationId = "2";
    requestParams.stage = "p2p";
    requestParams.operatorName = "hcom_send__822_0";
    database->QueryDurationList(requestParams, durationList);
    int expectSize = 2;
    EXPECT_EQ(durationList.size(), expectSize);
}

TEST_F(TestSuit, QueryDurationDataWithRank)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::DurationListParams requestParams;
    std::vector<Dic::Module::DurationDo> durationList;
    requestParams.dbIndex = "0";
    requestParams.iterationId = "2";
    requestParams.stage = "p2p";
    requestParams.operatorName = "hcom_send__822_0";
    requestParams.rankList = {"0"};
    database->QueryDurationList(requestParams, durationList);
    int expectSize = 1;
    EXPECT_EQ(durationList.size(), expectSize);
}

TEST_F(TestSuit, QueryBandwidthDistributionData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::DistributionDataParam requestParams;
    Dic::Protocol::DistributionResBody responseBody;
    requestParams.iterationId = "2";
    requestParams.stage = "(0, 1, 2, 3, 4, 5, 6, 7)";
    requestParams.operatorName = "hcom_broadcast__483_1";
    requestParams.transportType = "HCCS";
    requestParams.rankId = "1";
    database->QueryDistributionData(requestParams, responseBody);
    std::string expectResult = "{\"0.016512\":[7,0.01114],\"0.015504\":[1,0.00166]}";
    EXPECT_EQ(responseBody.distributionData, expectResult);
}

TEST_F(TestSuit, QueryBandwidthData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::BandwidthDataParam requestParams;
    Dic::Protocol::BandwidthDataResBody responseBody;
    requestParams.iterationId = "2";
    requestParams.stage = "p2p";
    requestParams.operatorName = "Total Op Info";
    requestParams.rankId = "1";
    database->QueryBandwidthData(requestParams, responseBody);
    int expectSize = 4;
    EXPECT_EQ(responseBody.items.size(), expectSize);
    EXPECT_EQ(responseBody.items[0].transportType, "RDMA");
    EXPECT_EQ(responseBody.items[0].transitSize, 20.9715); // transitSize = 20.9715
    EXPECT_EQ(responseBody.items[0].transitTime, 0.8638); // transitTime = 0.8638
    EXPECT_EQ(responseBody.items[0].bandwidth, 24.2777); // bandwidth = 24.2777
    EXPECT_EQ(responseBody.items[0].largePacketRatio, 0.0);
}

TEST_F(TestSuit, QueryBandwidthDataWithErrorParamReturnExpectSize0)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::BandwidthDataParam requestParams;
    Dic::Protocol::BandwidthDataResBody responseBody;
    requestParams.iterationId = "100";
    requestParams.stage = "p2p";
    requestParams.operatorName = "Total Op Info";
    requestParams.rankId = "1";
    database->QueryBandwidthData(requestParams, responseBody);
    const int expectSize = 0;
    EXPECT_EQ(responseBody.items.size(), expectSize);
}

TEST_F(TestSuit, QueryOperatorsCount)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::OperatorDetailsParam requestParams;
    Dic::Protocol::OperatorDetailsResBody responseBody;
    requestParams.iterationId = "2";
    requestParams.stage = "p2p";
    requestParams.rankId = "1";
    database->QueryOperatorsCount(requestParams, responseBody);
    const int expectSize = 1;
    EXPECT_EQ(responseBody.count, expectSize);
    requestParams.iterationId = "2";
    requestParams.stage = "(0, 1, 2, 3, 4, 5, 6, 7)";
    requestParams.rankId = "1";
    database->QueryOperatorsCount(requestParams, responseBody);
    const int stageExpectSize = 2;
    EXPECT_EQ(responseBody.count, stageExpectSize);
}

TEST_F(TestSuit, GetCommunicationGroups)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::string iterationId = "2";
    std::vector<std::string> groupList;
    database->GetGroups(iterationId, groupList);
    int expectSize = 2;
    EXPECT_EQ(groupList.size(), expectSize);
    EXPECT_EQ(groupList[0], "p2p");
    EXPECT_EQ(groupList[1], "(0, 1, 2, 3, 4, 5, 6, 7)");
}

TEST_F(TestSuit, QueryMatrixData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::MatrixBandwidthParam requestParams;
    std::vector<Dic::Module::MatrixInfoDo> matrixList;
    requestParams.iterationId = "2";
    requestParams.stage = "p2p";
    requestParams.operatorName = "Total Op Info";
    database->QueryMatrixList(requestParams, matrixList);
    int expectSize = 2;
    EXPECT_EQ(matrixList.size(), expectSize);
    EXPECT_EQ(matrixList[1].srcRank, 0);
    EXPECT_EQ(matrixList[1].dstRank, 8); // dstRank = 8
    EXPECT_EQ(matrixList[1].transportType, "RDMA");
    EXPECT_EQ(matrixList[1].transitSize, 20.9715); // transitSize = 20.9715
    EXPECT_EQ(matrixList[1].transitTime, 0.8638); // transitTime = 0.8638
    EXPECT_EQ(matrixList[1].bandwidth, 24.2778); // bandwidth = 24.2778
}

TEST_F(TestSuit, QueryAllCommunicationOperatorsDetails)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::OperatorDetailsParam requestParams;
    Dic::Protocol::OperatorDetailsResBody responseBody;
    requestParams.iterationId = "2";
    requestParams.stage = "p2p";
    requestParams.rankId = "1";
    requestParams.currentPage = 1;
    requestParams.pageSize = 10; // pageSize = 10
    database->QueryAllOperators(requestParams, responseBody);
    int expectSize = 1;
    EXPECT_EQ(responseBody.allOperators.size(), expectSize);
    EXPECT_EQ(responseBody.allOperators[0].operatorName, "hcom_send__822_0");
    EXPECT_EQ(responseBody.allOperators[0].elapseTime, 0.8966); // elapseTime = 0.8966
    EXPECT_EQ(responseBody.allOperators[0].transitTime, 0.8638); // transitTime = 0.8638
    EXPECT_EQ(responseBody.allOperators[0].synchronizationTime, 0);
    EXPECT_EQ(responseBody.allOperators[0].waitTime, 0);
    EXPECT_EQ(responseBody.allOperators[0].idleTime, 0.0327); // idleTime = 0.0327
    EXPECT_EQ(responseBody.allOperators[0].synchronizationTimeRatio, 0.0);
    EXPECT_EQ(responseBody.allOperators[0].waitTimeRatio, 0.0);
}

TEST_F(TestSuit, QueryMatrixSortOpNames)
{
    DataBaseManager::Instance().SetDataType(DataType::TEXT);
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::vector<Dic::Protocol::OperatorNamesObject> responseBody;
    Dic::Protocol::OperatorNamesParams requestParams;
    requestParams.iterationId = "2";
    requestParams.stage = "p2p";
    requestParams.rankList = {};
    database->QueryMatrixSortOpNames(requestParams, responseBody);
    int expectSize = 3;
    EXPECT_EQ(responseBody.size(), expectSize);
}

TEST_F(TestSuit, QueryAllPerformanceDataByStepWhenSingleStep)
{
    DataBaseManager::Instance().SetDataType(DataType::TEXT);
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::string step = "2";
    std::unordered_map<std::uint32_t, Dic::Module::StepStatistic> data{};
    auto result = database->QueryAllPerformanceDataByStep(step, data);
    EXPECT_EQ(result, true);
    EXPECT_EQ(data.size(), 16); // 16
    EXPECT_EQ(data.at(0).prepareTime, 0);
    step = "3";
    data.clear();
    result = database->QueryAllPerformanceDataByStep(step, data);
    EXPECT_EQ(result, true);
    EXPECT_EQ(data.size(), 0);
}

TEST_F(TestSuit, QueryAllPerformanceDataByStepWhenAllStep)
{
    DataBaseManager::Instance().SetDataType(DataType::TEXT);
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::string step = "";
    std::unordered_map<std::uint32_t, Dic::Module::StepStatistic> data{};
    auto result = database->QueryAllPerformanceDataByStep(step, data);
    EXPECT_EQ(result, true);
    EXPECT_EQ(data.size(), 16); // 16
    EXPECT_EQ(data.at(0).prepareTime, 0);
    EXPECT_NEAR(data.at(0).freeTime, 69909.504, 0.0001); // 69909.504 for result, 0.0001 for error
    step = "All";
    data.clear();
    result = database->QueryAllPerformanceDataByStep(step, data);
    EXPECT_EQ(result, true);
    EXPECT_EQ(data.size(), 16); // 16
    EXPECT_EQ(data.at(0).prepareTime, 0);
    EXPECT_NEAR(data.at(0).freeTime, 69909.504, 0.0001); // 69909.504 for result, 0.0001 for error
}
