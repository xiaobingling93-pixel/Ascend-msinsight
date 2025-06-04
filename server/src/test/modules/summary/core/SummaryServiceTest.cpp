/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "SummaryService.h"
#include "ConstantDefs.h"
#include "ClusterFileParser.h"
#include "DataBaseManager.h"
#include "TimeUtil.h"
#include "BaselineManager.h"

using namespace Dic::Module::Summary;
const int NUMBER_ZERO = 0;
const int NUMBER_SIXTEEN = 16;

class SummaryServiceTest : public ::testing::Test {
protected:
    std::string filePath;

    void SetUp() override
    {
        std::string  currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        filePath = currPath + R"(/src/test/test_data/cluster_analysis_output)";
    }

    static std::string InitParser(const std::string &dataPath, const std::string &uniqueKey)
    {
        // 创建新的db连接对象
        auto database = Dic::Module::FullDb::DataBaseManager::Instance().CreateClusterDatabase(uniqueKey,
            Dic::Module::Timeline::DataType::TEXT);
        // 集群解析，如果集群已解析，则只会初始化db，然后结束流程
        Dic::Module::FullDb::ClusterFileParser clusterFileParser(dataPath, database,
                                                                 uniqueKey + Dic::TimeUtil::Instance().NowStr());
        clusterFileParser.ParseClusterFiles();
        clusterFileParser.ParseClusterStep2Files();
        return database->GetDbPath();
    }

    static void Clear()
    {
        Dic::Module::FullDb::DataBaseManager::Instance().EraseClusterDb(Dic::COMPARE);
        Dic::Module::FullDb::DataBaseManager::Instance().EraseClusterDb(Dic::BASELINE);
    }
};

TEST_F(SummaryServiceTest, QueryCompareSummaryBaseInfoAllFail)
{
    Clear();
    SummaryTopRankRequest request;
    request.params.isCompare = false;
    SummaryTopRankResponse response;
    SummaryService::QueryCompareSummaryBaseInfo(request, response);
    EXPECT_EQ(response.body.baseInfo.compare.filePath, "");
    EXPECT_EQ(response.body.baseInfo.baseline.filePath, "");
}

TEST_F(SummaryServiceTest, QueryCompareSummaryBaseInfoAllSuccess)
{
    Clear();
    InitParser(filePath, Dic::COMPARE);
    InitParser(filePath, Dic::BASELINE);
    SummaryTopRankRequest request;
    request.params.isCompare = true;
    request.params.clusterPath = Dic::COMPARE;
    BaselineManager::Instance().SetBaselineClusterPath(Dic::BASELINE);
    SummaryTopRankResponse response;
    SummaryService::QueryCompareSummaryBaseInfo(request, response);
    EXPECT_EQ(response.body.baseInfo.compare.rankCount, NUMBER_SIXTEEN);
    EXPECT_EQ(response.body.baseInfo.baseline.rankCount, NUMBER_SIXTEEN);
    Clear();
}

TEST_F(SummaryServiceTest, QueryCompareSummaryBaseInfoOnlyCompareSuccess)
{
    Clear();
    InitParser(filePath, Dic::COMPARE);
    SummaryTopRankRequest request;
    request.params.isCompare = true;
    request.params.clusterPath = Dic::COMPARE;
    SummaryTopRankResponse response;
    SummaryService::QueryCompareSummaryBaseInfo(request, response);
    EXPECT_EQ(response.body.baseInfo.compare.rankCount, NUMBER_SIXTEEN);
    EXPECT_EQ(response.body.baseInfo.baseline.rankCount, NUMBER_ZERO);
    Clear();
}

TEST_F(SummaryServiceTest, QueryCompareSummaryBaseInfoOnlyBaselineSuccess)
{
    Clear();
    InitParser(filePath, Dic::BASELINE);
    SummaryTopRankRequest request;
    request.params.isCompare = true;
    BaselineManager::Instance().SetBaselineClusterPath(Dic::BASELINE);
    SummaryTopRankResponse response;
    SummaryService::QueryCompareSummaryBaseInfo(request, response);
    EXPECT_EQ(response.body.baseInfo.compare.rankCount, NUMBER_ZERO);
    EXPECT_EQ(response.body.baseInfo.baseline.rankCount, NUMBER_SIXTEEN);
    Clear();
}

TEST_F(SummaryServiceTest, QueryCompareSummaryParallelStrategyWithAlgIsNull)
{
    Clear();
    InitParser(filePath, Dic::BASELINE);
    InitParser(filePath, Dic::COMPARE);
    ParallelismPerformance params;
    params.isCompare = true;
    params.clusterPath = Dic::COMPARE;
    BaselineManager::Instance().SetBaselineClusterPath(Dic::COMPARE);
    PerformanceIndicatorData indicatorData;
    SummaryService::QueryParallelismPerformanceInfo(params, indicatorData);
    EXPECT_EQ(indicatorData.indicators.size(), NUMBER_ZERO);
}

TEST_F(SummaryServiceTest, QueryCompareSummaryParallelStrategySuccessWhenIsCompare)
{
    Clear();
    std::string dbPath = InitParser(filePath, Dic::BASELINE);
    InitParser(filePath, Dic::COMPARE);
    Dic::Module::ParallelStrategyConfig config;
    const int defaultSize = 2;
    config.ppSize = defaultSize;
    config.tpSize = defaultSize;
    config.dpSize = defaultSize;
    config.cpSize = defaultSize;
    config.epSize = defaultSize;
    std::string err;
    ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(dbPath, config, err);
    ParallelismPerformance params;
    params.dimension = "ep-dp-pp-cp-tp";
    params.step = "2";
    params.isCompare = true;
    params.config = config;
    params.clusterPath = Dic::COMPARE;
    BaselineManager::Instance().SetBaselineClusterPath(Dic::BASELINE);
    PerformanceIndicatorData indicatorData;
    SummaryService::QueryParallelismPerformanceInfo(params, indicatorData);
    EXPECT_EQ(indicatorData.performanceData.size(), NUMBER_SIXTEEN);
}

TEST_F(SummaryServiceTest, QueryCompareSummaryParallelStrategySuccessWithDpDimWhenIsNotCompare)
{
    Clear();
    std::string dbPath = InitParser(filePath, Dic::BASELINE);
    InitParser(filePath, Dic::COMPARE);
    Dic::Module::ParallelStrategyConfig config;
    const int defaultSize = 2;
    config.ppSize = defaultSize;
    config.tpSize = defaultSize;
    config.dpSize = defaultSize;
    config.cpSize = defaultSize;
    config.epSize = defaultSize;
    std::string err;
    ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(dbPath, config, err);
    ParallelismPerformance params;
    params.dimension = "ep-dp";
    params.step = "2";
    params.isCompare = false;
    params.config = config;
    params.clusterPath = Dic::COMPARE;
    auto algorithm = ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName(dbPath);
    algorithm->UpdateParallelDimension(params.dimension, config, err);
    PerformanceIndicatorData indicatorData;
    SummaryService::QueryParallelismPerformanceInfo(params, indicatorData);
    EXPECT_EQ(indicatorData.performanceData.size(), 2); // 2 for dpSize
}

TEST_F(SummaryServiceTest, QueryCompareSummaryParallelStrategySuccessWithPpDimWhenIsNotCompare)
{
    Clear();
    std::string dbPath = InitParser(filePath, Dic::BASELINE);
    InitParser(filePath, Dic::COMPARE);
    Dic::Module::ParallelStrategyConfig config;
    const int defaultSize = 2;
    config.ppSize = defaultSize;
    config.tpSize = defaultSize;
    config.dpSize = defaultSize;
    config.cpSize = defaultSize;
    config.epSize = defaultSize;
    std::string err;
    ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(dbPath, config, err);
    ParallelismPerformance params;
    params.dimension = "ep-dp-pp";
    params.step = "2";
    params.isCompare = false;
    params.config = config;
    params.clusterPath = Dic::COMPARE;
    auto algorithm = ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName(dbPath);
    algorithm->UpdateParallelDimension(params.dimension, config, err);
    PerformanceIndicatorData indicatorData;
    SummaryService::QueryParallelismPerformanceInfo(params, indicatorData);
    EXPECT_EQ(indicatorData.performanceData.size(), 4); // 4 for dpSize * ppSize
}

TEST_F(SummaryServiceTest, QueryCompareSummaryParallelStrategySuccessWithCpDimWhenIsNotCompare)
{
    Clear();
    std::string dbPath = InitParser(filePath, Dic::BASELINE);
    InitParser(filePath, Dic::COMPARE);
    Dic::Module::ParallelStrategyConfig config;
    const int defaultSize = 2;
    config.ppSize = defaultSize;
    config.tpSize = defaultSize;
    config.dpSize = defaultSize;
    config.cpSize = defaultSize;
    config.epSize = defaultSize;
    std::string err;
    ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(dbPath, config, err);
    ParallelismPerformance params;
    params.dimension = "ep-dp-pp-cp";
    params.step = "2";
    params.isCompare = false;
    params.config = config;
    params.clusterPath = Dic::COMPARE;
    auto algorithm = ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName(dbPath);
    algorithm->UpdateParallelDimension(params.dimension, config, err);
    PerformanceIndicatorData indicatorData;
    SummaryService::QueryParallelismPerformanceInfo(params, indicatorData);
    EXPECT_EQ(indicatorData.performanceData.size(), 8); // 8 for dpSize * ppSize * cpSize
}


TEST_F(SummaryServiceTest, QueryCompareSummaryParallelStrategySuccessWithTpDimWhenIsNotCompare)
{
    Clear();
    std::string dbPath = InitParser(filePath, Dic::BASELINE);
    InitParser(filePath, Dic::COMPARE);
    Dic::Module::ParallelStrategyConfig config;
    const int defaultSize = 2;
    config.ppSize = defaultSize;
    config.tpSize = defaultSize;
    config.dpSize = defaultSize;
    config.cpSize = defaultSize;
    config.epSize = defaultSize;
    std::string err;
    ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(dbPath, config, err);
    ParallelismPerformance params;
    params.dimension = "ep-dp-pp-cp-tp";
    params.step = "2";
    params.isCompare = false;
    params.config = config;
    params.clusterPath = Dic::COMPARE;
    auto algorithm = ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName(dbPath);
    algorithm->UpdateParallelDimension(params.dimension, config, err);
    PerformanceIndicatorData indicatorData;
    SummaryService::QueryParallelismPerformanceInfo(params, indicatorData);
    EXPECT_EQ(indicatorData.performanceData.size(), 16); // 16 for dpSize * ppSize * cpSize * tpSize
}