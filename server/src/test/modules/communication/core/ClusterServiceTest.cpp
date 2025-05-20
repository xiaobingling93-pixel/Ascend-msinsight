/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "ClusterService.h"
#include "ConstantDefs.h"
#include "ClusterFileParser.h"
#include "DataBaseManager.h"
#include "TimeUtil.h"
#include "BaselineManager.h"

using namespace Dic::Module::Communication;

const int NUMBER_ZERO = 0;
const int NUMBER_ONE = 1;
const int NUMBER_TWO = 2;
const int NUMBER_TWENTY_EIGHT = 28;

class ClusterServiceTest : public ::testing::Test {
protected:
    std::string filePath;

    void SetUp() override
    {
        std::string  currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        filePath = currPath + R"(/src/test/test_data/cluster_analysis_output)";
    }

    static void InitParser(const std::string &dataPath, const std::string &uniqueKey)
    {
        // 创建新的db连接对象
        auto database = Dic::Module::FullDb::DataBaseManager::Instance().CreateClusterDatabase(uniqueKey,
            Dic::Module::Timeline::DataType::TEXT);
        // 集群解析，如果集群已解析，则只会初始化db，然后结束流程
        Dic::Module::FullDb::ClusterFileParser clusterFileParser(dataPath, database,
                                                                 uniqueKey + Dic::TimeUtil::Instance().NowStr());
        clusterFileParser.ParseClusterFiles();
        clusterFileParser.ParseClusterStep2Files();
    }

    static void Clear()
    {
        Dic::Module::FullDb::DataBaseManager::Instance().EraseClusterDb(Dic::COMPARE);
        Dic::Module::FullDb::DataBaseManager::Instance().EraseClusterDb(Dic::BASELINE);
    }
};

TEST_F(ClusterServiceTest, QueryIterationsAllFail)
{
    Clear();
    Dic::Protocol::IterationsRequest request;
    request.params.isCompare = false;
    Dic::Protocol::IterationsOrRanksResponse response;
    ClusterService::QueryIterations(request, response);
    EXPECT_EQ(response.body.compare.size(), NUMBER_ZERO);
    EXPECT_EQ(response.body.baseline.size(), NUMBER_ZERO);
}

TEST_F(ClusterServiceTest, QueryIterationsAllSuccess)
{
    Clear();
    InitParser(filePath, Dic::COMPARE);
    InitParser(filePath, Dic::BASELINE);
    Dic::Protocol::IterationsRequest request;
    request.params.isCompare = true;
    request.params.clusterPath = Dic::COMPARE;
    BaselineManager::Instance().SetBaselineClusterPath(Dic::BASELINE);
    Dic::Protocol::IterationsOrRanksResponse response;
    ClusterService::QueryIterations(request, response);
    EXPECT_EQ(response.body.compare.size(), NUMBER_ONE);
    EXPECT_EQ(response.body.compare[0].iterationOrRankId, "2");
    EXPECT_EQ(response.body.baseline.size(), NUMBER_ONE);
    Clear();
}

TEST_F(ClusterServiceTest, QueryIterationsOnlyCompareSuccess)
{
    Clear();
    InitParser(filePath, Dic::COMPARE);
    Dic::Protocol::IterationsRequest request;
    request.params.isCompare = true;
    request.params.clusterPath = Dic::COMPARE;
    Dic::Protocol::IterationsOrRanksResponse response;
    ClusterService::QueryIterations(request, response);
    EXPECT_EQ(response.body.compare.size(), NUMBER_ONE);
    EXPECT_EQ(response.body.baseline.size(), NUMBER_ZERO);
    Clear();
}

TEST_F(ClusterServiceTest, QueryGroupInfoAllFail)
{
    Clear();
    Dic::Protocol::MatrixGroupRequest request;
    request.params.isCompare = false;
    Dic::Protocol::MatrixGroupResponse response;
    ClusterService::QueryGroupInfo(request, response);
    EXPECT_EQ(response.body.groupList.size(), NUMBER_ZERO);
    EXPECT_EQ(response.body.groupList.size(), NUMBER_ZERO);
}

TEST_F(ClusterServiceTest, QueryGroupInfoAllSuccess)
{
    InitParser(filePath, Dic::COMPARE);
    InitParser(filePath, Dic::BASELINE);
    Dic::Protocol::MatrixGroupRequest request;
    request.params.iterationId = "2";
    request.params.baselineIterationId = "2";
    request.params.isCompare = true;
    request.params.clusterPath = Dic::COMPARE;
    BaselineManager::Instance().SetBaselineClusterPath(Dic::BASELINE);
    Dic::Protocol::MatrixGroupResponse response;
    ClusterService::QueryGroupInfo(request, response);
    EXPECT_EQ(response.body.groupList.size(), NUMBER_TWENTY_EIGHT);
    EXPECT_EQ(response.body.groupList[0].type, "common");
    Clear();
}

TEST_F(ClusterServiceTest, QueryGroupInfoOnlyCompareSuccess)
{
    InitParser(filePath, Dic::COMPARE);
    Dic::Protocol::MatrixGroupRequest request;
    request.params.iterationId = "2";
    request.params.baselineIterationId = "1";
    request.params.isCompare = true;
    request.params.clusterPath = Dic::COMPARE;
    Dic::Protocol::MatrixGroupResponse response;
    ClusterService::QueryGroupInfo(request, response);
    EXPECT_EQ(response.body.groupList.size(), NUMBER_TWENTY_EIGHT);
    EXPECT_EQ(response.body.groupList[0].type, "compare");
    Clear();
}

TEST_F(ClusterServiceTest, QueryMatrixInfoSuccess)
{
    InitParser(filePath, Dic::COMPARE);
    InitParser(filePath, Dic::BASELINE);
    Dic::Protocol::MatrixBandwidthParam params = {"(0, 1, 2, 3, 4, 5, 6, 7)", "hcom_broadcast__483_0",
                                                  "2", "", true, "2", Dic::COMPARE};
    BaselineManager::Instance().SetBaselineClusterPath(Dic::BASELINE);
    Dic::Protocol::MatrixListResponseBody body;
    ClusterService::QueryMatrixInfo(params, body);
    EXPECT_EQ(body.matrixList.size(), NUMBER_TWO);
    EXPECT_EQ(body.matrixList[0].srcRank, NUMBER_ZERO);
    Clear();
}

TEST_F(ClusterServiceTest, QueryMatrixInfoFailOfDatabaseNotExist)
{
    Clear();
    Dic::Protocol::MatrixBandwidthParam params = {"(0, 1, 2, 3, 4, 5, 6, 7)", "hcom_broadcast__483_0",
                                                  "2", "", true, "2", Dic::COMPARE};
    Dic::Protocol::MatrixListResponseBody body;
    ClusterService::QueryMatrixInfo(params, body);
    EXPECT_EQ(body.matrixList.size(), NUMBER_ZERO);
}

TEST_F(ClusterServiceTest, QueryOperatorListSuccess)
{
    InitParser(filePath, Dic::COMPARE);
    InitParser(filePath, Dic::BASELINE);
    Dic::Protocol::DurationListParams params;
    params.operatorName = "hcom_broadcast__483_1";
    params.iterationId = "2";
    params.stage = "(0, 1, 2, 3, 4, 5, 6, 7)";
    params.baselineIterationId = "2";
    params.isCompare = true;
    params.clusterPath = Dic::COMPARE;
    BaselineManager::Instance().SetBaselineClusterPath(Dic::BASELINE);
    Dic::Protocol::OperatorListsResponseBody body;
    ClusterService::QueryOperatorList(params, body);
    const uint64_t expectMaxTime = 11229980;
    const uint64_t expectMinTime = 10397500;
    EXPECT_EQ(body.opLists.size(), NUMBER_TWO);
    EXPECT_EQ(body.maxTime, expectMaxTime);
    EXPECT_EQ(body.minTime, expectMinTime);
    EXPECT_EQ(body.opLists[0].compare.size(), NUMBER_ONE);
    EXPECT_EQ(body.opLists[0].baseline.size(), NUMBER_ONE);
    Clear();
}

TEST_F(ClusterServiceTest, QueryOperatorListFailWithoutDb)
{
    Clear();
    Dic::Protocol::DurationListParams params;
    params.operatorName = "hcom_broadcast__483_1";
    params.iterationId = "2";
    params.stage = "(0, 1, 2, 3, 4, 5, 6, 7)";
    params.isCompare = true;
    params.clusterPath = Dic::COMPARE;
    Dic::Protocol::OperatorListsResponseBody body;
    ClusterService::QueryOperatorList(params, body);
    EXPECT_EQ(body.opLists.size(), NUMBER_ZERO);
}

TEST_F(ClusterServiceTest, QueryDurationListSuccess)
{
    InitParser(filePath, Dic::COMPARE);
    InitParser(filePath, Dic::BASELINE);
    Dic::Protocol::DurationListParams params;
    params.operatorName = "hcom_broadcast__483_1";
    params.iterationId = "2";
    params.stage = "(0, 1, 2, 3, 4, 5, 6, 7)";
    params.baselineIterationId = "2";
    params.isCompare = true;
    params.clusterPath = Dic::COMPARE;
    BaselineManager::Instance().SetBaselineClusterPath(Dic::BASELINE);
    Dic::Protocol::DurationListsResponseBody body;
    ClusterService::QueryDurationList(params, body);
    const double expectStartTime = 10.6855;
    EXPECT_EQ(body.durationList.size(), NUMBER_TWO);
    EXPECT_EQ(body.durationList[0].rankId, "0");
    EXPECT_EQ(body.durationList[0].durationData.compare.startTime, expectStartTime);
    EXPECT_EQ(body.durationList[0].durationData.baseline.startTime, expectStartTime);
    EXPECT_EQ(body.bwStatistics.size(), NUMBER_ONE);
    Clear();
}

TEST_F(ClusterServiceTest, QueryDurationListFailWithoutDb)
{
    Clear();
    Dic::Protocol::DurationListParams params;
    params.operatorName = "hcom_broadcast__483_1";
    params.iterationId = "2";
    params.stage = "(0, 1, 2, 3, 4, 5, 6, 7)";
    params.baselineIterationId = "2";
    params.isCompare = true;
    params.clusterPath = Dic::COMPARE;
    Dic::Protocol::DurationListsResponseBody body;
    ClusterService::QueryDurationList(params, body);
    EXPECT_EQ(body.durationList.size(), NUMBER_ZERO);
}