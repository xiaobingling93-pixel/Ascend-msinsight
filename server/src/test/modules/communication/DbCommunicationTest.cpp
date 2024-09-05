/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MemoryProtocolRequest.h"
#include "DataBaseManager.h"
#include "DbClusterDataBase.h"
#include "ParamsParser.h"
#include "FileUtil.h"
#include "TraceTime.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;

class DbCommunicationTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        const ParamsOption &option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string dbPath3 = R"(/src/test/test_data/full_db/)";
        DataBaseManager::Instance().SetDataType(DataType::DB);
        DataBaseManager::Instance().SetFileType(FileType::MS_PROF);
        auto culsterDatabase =
                dynamic_cast<DbClusterDataBase *>(DataBaseManager::Instance().GetReadClusterDatabase());
        culsterDatabase->OpenDb(currPath + dbPath3 + "cluster_analysis.db", false);
    }
    static void TearDownTestCase() {}
};

TEST_F(DbCommunicationTest, QueryIterationsData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    std::vector<Dic::Protocol::IterationsOrRanksObject> responseBody;
    database->QueryIterations(responseBody);
    int expectSize = 2;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(responseBody[0].iterationOrRankId, "1");
}

TEST_F(DbCommunicationTest, QueryOperatorNameData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
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
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
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
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
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
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    Dic::Protocol::DurationListParams requestParams;
    Protocol::DurationListsResponseBody responseBody;
    requestParams.dbIndex = "0";
    requestParams.iterationId = "1";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    requestParams.operatorName = "Total Op Info";
    database->QueryDurationList(requestParams, responseBody);
    int expectSize = 8;
    EXPECT_EQ(responseBody.durationList.size(), expectSize);
}

TEST_F(DbCommunicationTest, QueryDurationDataWithRank)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    Dic::Protocol::DurationListParams requestParams;
    Protocol::DurationListsResponseBody responseBody;
    requestParams.dbIndex = "0";
    requestParams.iterationId = "1";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    requestParams.operatorName = "Total Op Info";
    requestParams.rankList = {"0"};
    database->QueryDurationList(requestParams, responseBody);
    int expectSize = 1;
    EXPECT_EQ(responseBody.durationList.size(), expectSize);
}

TEST_F(DbCommunicationTest, QueryBandwidthDistributionData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
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
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
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

TEST_F(DbCommunicationTest, QueryBandwidthDataWithErrorParamReturnExpectSize0)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
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
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
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
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    Dic::Protocol::MatrixGroupParam requestParams;
    Dic::Protocol::MatrixGroupResponseBody responseBody;
    requestParams.iterationId = "1";
    database->GetGroups(requestParams, responseBody);
    int expectSize = 1;
    EXPECT_EQ(responseBody.groupList.size(), expectSize);
    EXPECT_EQ(responseBody.groupList[0], "(0,1,2,3,4,5,6,7)");
}

TEST_F(DbCommunicationTest, QueryMatrixData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    Dic::Protocol::MatrixBandwidthParam requestParams;
    Dic::Protocol::MatrixListResponseBody responseBody;
    requestParams.iterationId = "1";
    requestParams.stage = "(0,1,2,3,4,5,6,7)";
    requestParams.operatorName = "allgather-bottom1";
    database->QueryMatrixList(requestParams, responseBody);
    int expectSize = 64;
    EXPECT_EQ(responseBody.matrixList.size(), expectSize);
}

TEST_F(DbCommunicationTest, QueryAllCommunicationOperatorsDetails)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
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