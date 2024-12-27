/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MemoryProtocolRequest.h"
#include "DataBaseManager.h"
#include "DbClusterDataBase.h"
#include "ClusterDomainObject.h"
#include "ParamsParser.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;
using namespace Dic;

class DbCommunicationTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        const ParamsOption &option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string dbPath3 = R"(/src/test/test_data/full_db/)";
        DataBaseManager::Instance().SetDataType(DataType::DB);
        DataBaseManager::Instance().SetFileType(FileType::MS_PROF);
        auto culsterDatabase = DataBaseManager::Instance().CreateClusterDatabase(COMPARE, DataType::DB);
        culsterDatabase->OpenDb(currPath + dbPath3 + "cluster_analysis.db", false);
    }
    static void TearDownTestSuite() {}
};

TEST_F(DbCommunicationTest, QueryIterationsData)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::vector<Dic::Protocol::IterationsOrRanksObject> responseBody;
    database->QueryIterations(responseBody);
    int expectSize = 2;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(responseBody[0].iterationOrRankId, "1");
}

TEST_F(DbCommunicationTest, QueryOperatorNameData)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::OperatorNamesParams requestParams;
    requestParams.dbIndex = "0";
    requestParams.iterationId = "1";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    requestParams.rankList = {};
    std::vector<Dic::Protocol::OperatorNamesObject> responseBody;
    database->QueryOperatorNames(requestParams, responseBody);
    int expectSize = 1325;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(responseBody[0].operatorName, "Total Op Info");
    EXPECT_EQ(responseBody[1].operatorName, "hcom_allGather__018_3_1");
}

TEST_F(DbCommunicationTest, QueryOperatorNameDataWithRank)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::OperatorNamesParams requestParams;
    requestParams.dbIndex = "0";
    requestParams.iterationId = "1";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    requestParams.rankList = {"0"};
    std::vector<Dic::Protocol::OperatorNamesObject> responseBody;
    database->QueryOperatorNames(requestParams, responseBody);
    int expectSize = 1325;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(responseBody[0].operatorName, "Total Op Info");
    EXPECT_EQ(responseBody[1].operatorName, "hcom_allGather__018_3_1");
}

TEST_F(DbCommunicationTest, QueryRanksData)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::RanksParams requestParam;
    requestParam.dbIndex = "0";
    requestParam.iterationId = "2";
    std::vector<Dic::Protocol::IterationsOrRanksObject> responseBody;
    database->QueryRanksHandler(responseBody);
    int expectSize = 8;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(responseBody[0].iterationOrRankId, "0");
    EXPECT_EQ(responseBody[1].iterationOrRankId, "1");
}

TEST_F(DbCommunicationTest, QueryDurationData)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::DurationListParams requestParams;
    std::vector<Dic::Module::DurationDo> durationDoList;
    requestParams.dbIndex = "0";
    requestParams.iterationId = "1";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    requestParams.operatorName = "Total Op Info";
    database->QueryDurationList(requestParams, durationDoList);
    int expectSize = 8;
    EXPECT_EQ(durationDoList.size(), expectSize);
}

TEST_F(DbCommunicationTest, QueryDurationDataWithRank)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::DurationListParams requestParams;
    std::vector<Dic::Module::DurationDo> durationDoList;
    requestParams.dbIndex = "0";
    requestParams.iterationId = "1";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    requestParams.operatorName = "Total Op Info";
    requestParams.rankList = {"0"};
    database->QueryDurationList(requestParams, durationDoList);
    int expectSize = 1;
    EXPECT_EQ(durationDoList.size(), expectSize);
}

TEST_F(DbCommunicationTest, QueryBandwidthDistributionData)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::DistributionDataParam requestParams;
    Dic::Protocol::DistributionResBody responseBody;
    requestParams.iterationId = "1";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    requestParams.operatorName = "hcom_allGather__018_3_1";
    requestParams.transportType = "HCCS";
    requestParams.rankId = "1";
    database->QueryDistributionData(requestParams, responseBody);
    std::string expectResult = "{\"0.0001\":[7,0.0100401953125]}";
    EXPECT_EQ(responseBody.distributionData, expectResult);
}

TEST_F(DbCommunicationTest, QueryBandwidthData)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::BandwidthDataParam requestParams;
    Dic::Protocol::BandwidthDataResBody responseBody;
    requestParams.iterationId = "1";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    requestParams.operatorName = "Total Op Info";
    requestParams.rankId = "1";
    database->QueryBandwidthData(requestParams, responseBody);
    int expectSize = 2;
    EXPECT_EQ(responseBody.items.size(), expectSize);
    EXPECT_EQ(responseBody.items[0].transportType, "HCCS");
}

TEST_F(DbCommunicationTest, QueryIterationAndCommunicationGroupDBSuccess)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::CommunicationKernelParams requestParams;
    requestParams.name = "hcom_broadcast__293_0_1";
    requestParams.rankId = "0";
    Dic::Protocol::CommunicationKernelBody responseBody;
    database->QueryIterationAndCommunicationGroup(requestParams, responseBody);
    EXPECT_EQ(responseBody.step, "1");
    EXPECT_EQ(responseBody.group, "(0,1,2,3,4,5,6,7)");
}

TEST_F(DbCommunicationTest, QueryBandwidthDataWithErrorParamReturnExpectSize0)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::BandwidthDataParam requestParams;
    Dic::Protocol::BandwidthDataResBody responseBody;
    requestParams.iterationId = "2";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    requestParams.operatorName = "Total Op Info";
    requestParams.rankId = "1";
    database->QueryBandwidthData(requestParams, responseBody);
    const int expectSize = 2;
    EXPECT_EQ(responseBody.items.size(), expectSize);
}

TEST_F(DbCommunicationTest, QueryOperatorsCount)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::OperatorDetailsParam requestParams;
    Dic::Protocol::OperatorDetailsResBody responseBody;
    requestParams.iterationId = "1";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    requestParams.rankId = "1";
    database->QueryOperatorsCount(requestParams, responseBody);
    const int expectSize = 1324;
    EXPECT_EQ(responseBody.count, expectSize);
    requestParams.iterationId = "2";
    requestParams.stage = "(0, 1, 2, 3, 4, 5, 6, 7)";
    requestParams.rankId = "0";
    database->QueryOperatorsCount(requestParams, responseBody);
    const int stageExpectSize = 0;
    EXPECT_EQ(responseBody.count, stageExpectSize);
}

TEST_F(DbCommunicationTest, GetCommunicationGroups)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::string iterationId = "1";
    std::vector<std::string> groupList;
    database->GetGroups(iterationId, groupList);
    int expectSize = 1;
    EXPECT_EQ(groupList.size(), expectSize);
    EXPECT_EQ(groupList[0], "(0,1,2,3,4,5,6,7)");
}

TEST_F(DbCommunicationTest, QueryMatrixData)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::MatrixBandwidthParam requestParams;
    std::vector<Dic::Module::MatrixInfoDo> matrixList;
    requestParams.iterationId = "1";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    requestParams.operatorName = "allgather-bottom1";
    database->QueryMatrixList(requestParams, matrixList);
    int expectSize = 64;
    EXPECT_EQ(matrixList.size(), expectSize);
}

TEST_F(DbCommunicationTest, QueryAllCommunicationOperatorsDetails)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::OperatorDetailsParam requestParams;
    Dic::Protocol::OperatorDetailsResBody responseBody;
    requestParams.iterationId = "step1";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    requestParams.rankId = "0";
    requestParams.currentPage = 1;
    requestParams.pageSize = 100; // pageSize = 100
    database->QueryAllOperators(requestParams, responseBody);
    int expectSize = 100;
    EXPECT_EQ(responseBody.allOperators.size(), expectSize);
    EXPECT_EQ(responseBody.allOperators[0].operatorName, "hcom_allReduce__293_647_1");
}

TEST_F(DbCommunicationTest, QuerySummaryDataSuccess)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::SummaryTopRankParams requestParams;
    Dic::Protocol::SummaryTopRankResBody responseBody;
    requestParams.rankIdList.emplace_back("0");
    requestParams.stepIdList.emplace_back("1");
    requestParams.orderBy = "rankId";
    database->QuerySummaryData(requestParams, responseBody);
    int expectSize = 1;
    const double expectCommunicationOverLappedTime = 887304.62;
    EXPECT_EQ(responseBody.summaryList.size(), expectSize);
    EXPECT_EQ(responseBody.summaryList[0].rankId, "0");
    EXPECT_EQ(responseBody.summaryList[0].communicationOverLappedTime, expectCommunicationOverLappedTime);
}

TEST_F(DbCommunicationTest, QueryBaseInfoSuccess)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::SummaryTopRankResBody responseBody;
    database->QueryBaseInfo(responseBody.baseInfo.compare);
    const int expectRankCount = 8;
    const int expectStepNum = 2;
    EXPECT_EQ(responseBody.baseInfo.compare.rankCount, expectRankCount);
    EXPECT_EQ(responseBody.baseInfo.compare.stepNum, expectStepNum);
}

TEST_F(DbCommunicationTest, GetStepIdListSuccess)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::PipelineStepResponseBody responseBody;
    database->GetStepIdList(responseBody);
    const int expectSize = 2;
    EXPECT_EQ(responseBody.stepList.size(), expectSize);
}

TEST_F(DbCommunicationTest, GetStageAndBubbleSuccess)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::PipelineStageTimeParam requestParams;
    requestParams.stepId = "1";
    requestParams.stageId = "(0)";
    Dic::Protocol::PipelineStageOrRankTimeResponseBody responseBody;
    database->GetStageAndBubble(requestParams, responseBody);
    const int expectSize = 1;
    const double expectBubbleTime = 0;
    const double expectStageTime = 10635932.446;
    EXPECT_EQ(responseBody.bubbleDetails.size(), expectSize);
    EXPECT_EQ(responseBody.bubbleDetails[0].bubbleTime, expectBubbleTime);
    EXPECT_EQ(responseBody.bubbleDetails[0].stageTime, expectStageTime);
}

TEST_F(DbCommunicationTest, GetRankAndBubbleSuccess)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::PipelineRankTimeParam requestParams;
    requestParams.stepId = "1";
    requestParams.stageId = "(0)";
    Dic::Protocol::PipelineStageOrRankTimeResponseBody responseBody;
    database->GetRankAndBubble(requestParams, responseBody);
    const int expectSize = 1;
    const double expectBubbleTime = 0;
    const double expectStageTime = 10635932.446;
    EXPECT_EQ(responseBody.bubbleDetails.size(), expectSize);
    EXPECT_EQ(responseBody.bubbleDetails[0].bubbleTime, expectBubbleTime);
    EXPECT_EQ(responseBody.bubbleDetails[0].stageTime, expectStageTime);
}

TEST_F(DbCommunicationTest, QueryExtremumTimestampSuccess)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    uint64_t min = 0;
    uint64_t max = 0;
    bool res = database->QueryExtremumTimestamp(min, max);
    const uint64_t expectMin = 1718682999267391232;
    const uint64_t expectMax = 1718683020669117696;
    EXPECT_EQ(res, true);
    EXPECT_EQ(min, expectMin);
    EXPECT_EQ(max, expectMax);
}

TEST_F(DbCommunicationTest, QueryOperatorListSuccess)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::DurationListParams requestParams;
    requestParams.rankList.emplace_back("0");
    requestParams.operatorName = "hcom_broadcast__293_0_1";
    requestParams.iterationId = "1";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    std::vector<Dic::Module::OperatorTimeDo> opLists;
    database->QueryOperatorList(requestParams, opLists);
    int expectSize = 1;
    EXPECT_EQ(opLists.size(), expectSize);
}

TEST_F(DbCommunicationTest, QueryCommunicationGroupSuccess)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Document responseBody;
    bool res = database->QueryCommunicationGroup(responseBody);
    EXPECT_EQ(res, true);
}

TEST_F(DbCommunicationTest, QueryMatrixSortOpNamesSuccess)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Protocol::OperatorNamesParams requestParams;
    requestParams.rankList.emplace_back("0");
    requestParams.iterationId = "1";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    std::vector<Protocol::OperatorNamesObject> responseBody;
    bool res = database->QueryMatrixSortOpNames(requestParams, responseBody);
    const int exceptSize = 17;
    EXPECT_EQ(res, true);
    EXPECT_EQ(responseBody.size(), exceptSize);
}

TEST_F(DbCommunicationTest, GetParallelConfigFromStepTraceSuccess)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    Dic::Module::ParallelStrategyConfig config;
    std::string level;
    bool res = database->GetParallelConfigFromStepTrace(config, level);
    const int exceptTpSize = 1;
    EXPECT_EQ(res, true);
    EXPECT_EQ(config.tpSize, exceptTpSize);
    EXPECT_EQ(level, "undefined");
}

TEST_F(DbCommunicationTest, QueryAllPerformanceDataByStepWhenSingleStep)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::string step = "1";
    std::unordered_map<uint32_t, Dic::Module::StepStatistic> data{};
    auto result = database->QueryAllPerformanceDataByStep(step, data);
    EXPECT_EQ(result, true);
    EXPECT_EQ(data.size(), 8); // 8
    EXPECT_EQ(data.at(0).prepareTime, 473.646); // 473.646 for result
    step = "3";
    data.clear();
    result = database->QueryAllPerformanceDataByStep(step, data);
    EXPECT_EQ(result, true);
    EXPECT_EQ(data.size(), 0);
}

TEST_F(DbCommunicationTest, QueryAllPerformanceDataByStepWhenAllStep)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::string step = "";
    std::unordered_map<uint32_t, Dic::Module::StepStatistic> data{};
    auto result = database->QueryAllPerformanceDataByStep(step, data);
    EXPECT_EQ(result, true);
    EXPECT_EQ(data.size(), 8); // 8
    EXPECT_EQ(data.at(0).prepareTime, 1075.410); // 1075.410 for result
    step = "All";
    data.clear();
    result = database->QueryAllPerformanceDataByStep(step, data);
    EXPECT_EQ(result, true);
    EXPECT_EQ(data.size(), 8); // 8
    EXPECT_EQ(data.at(0).prepareTime, 1075.410); // 1075.410 for result
}