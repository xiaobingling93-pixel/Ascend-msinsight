/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MegatronParallelStrategyAlgorithm.h"
using namespace Dic::Module;
using namespace Dic::Protocol;
class MegatronParallelStrategyAlgorithmTest : public ::testing::Test {
};

TEST_F(MegatronParallelStrategyAlgorithmTest, UpdateParallelDimension_ShouldReturnTrue_WhenUpdateSuccess)
{
    MegatronParallelStrategyAlgorithm algorithm;
    std::string dimension = DIMENSIONS_TP;
    ParallelStrategyConfig config;
    config.ppSize = 2; // 2
    config.tpSize = 2; // 2
    config.dpSize = 4; // 4
    config.cpSize = 2; // 2
    config.epSize = 2; // 2
    config.algorithm = MEGATRON_LM_TP_CP_EP_DP_PP_ALG;
    std::string err;
    bool res = algorithm.UpdateParallelDimension(dimension, config, err);
    EXPECT_TRUE(res);
    EXPECT_EQ(algorithm.GetArrangementData().size, 32); // 32 = 2*2*4*2
    MegatronParallelStrategyAlgorithm algorithm2;
    config.algorithm = MEGATRON_LM_TP_CP_PP_EP_DP_ALG;
    res = algorithm2.UpdateParallelDimension(dimension, config, err);
    EXPECT_EQ(algorithm2.GetArrangementData().size, 32); // 32 = 2*2*4*2
    EXPECT_TRUE(res);
    MegatronParallelStrategyAlgorithm algorithm3;
    dimension = DIMENSIONS_CP;
    res = algorithm3.UpdateParallelDimension(dimension, config, err);
    EXPECT_EQ(algorithm3.GetArrangementData().size, 16); // 16 = 2*2*4
    EXPECT_TRUE(res);
    MegatronParallelStrategyAlgorithm algorithm4;
    dimension = DIMENSIONS_PP;
    res = algorithm4.UpdateParallelDimension(dimension, config, err);
    EXPECT_EQ(algorithm4.GetArrangementData().size, 8); // 8 = 2*4
    EXPECT_TRUE(res);
    MegatronParallelStrategyAlgorithm algorithm5;
    dimension = DIMENSIONS_DP;
    res = algorithm5.UpdateParallelDimension(dimension, config, err);
    EXPECT_EQ(algorithm5.GetArrangementData().size, 4); // 4
    EXPECT_TRUE(res);
}

TEST_F(MegatronParallelStrategyAlgorithmTest, UpdateParallelDimension_ShouldReturnFalse_WhenWrongInput)
{
    MegatronParallelStrategyAlgorithm algorithm;
    std::string dimension = "yyyyy";
    ParallelStrategyConfig config;
    config.ppSize = 2; // 2
    config.tpSize = 2; // 2
    config.dpSize = 4; // 4
    config.cpSize = 2; // 2
    config.epSize = 2; // 2
    config.algorithm = "xxxx";
    std::string err;
    bool res = algorithm.UpdateParallelDimension(dimension, config, err);
    EXPECT_FALSE(res);
    EXPECT_EQ(err, "Failed to update parallel view. Unexpected algorithm.");
    MegatronParallelStrategyAlgorithm algorithm2;
    config.algorithm = MEGATRON_LM_TP_DP_PP_ALG;
    res = algorithm.UpdateParallelDimension(dimension, config, err);
    EXPECT_FALSE(res);
    EXPECT_EQ(err, "Failed to update show map for parallel view. Unexpected dimension.");
}

TEST_F(MegatronParallelStrategyAlgorithmTest, GetArrangementByDimension_ShouldGetArrangement_TestWithTpDimension)
{
    MegatronParallelStrategyAlgorithm algorithm;
    std::string dimension = DIMENSIONS_TP;
    ParallelStrategyConfig config;
    config.ppSize = 2; // 2
    config.tpSize = 2; // 2
    config.dpSize = 2; // 2
    config.cpSize = 2; // 2
    config.epSize = 2; // 2
    config.algorithm = MEGATRON_LM_TP_CP_EP_DP_PP_ALG;
    const std::vector<std::string> EXPECTED_NAME = {
        "dp0-pp0-cp0-tp0", "dp0-pp0-cp0-tp1", "dp0-pp0-cp1-tp0", "dp0-pp0-cp1-tp1",
        "dp1-pp0-cp0-tp0", "dp1-pp0-cp0-tp1", "dp1-pp0-cp1-tp0", "dp1-pp0-cp1-tp1",
        "dp0-pp1-cp0-tp0", "dp0-pp1-cp0-tp1", "dp0-pp1-cp1-tp0", "dp0-pp1-cp1-tp1",
        "dp1-pp1-cp0-tp0", "dp1-pp1-cp0-tp1", "dp1-pp1-cp1-tp0", "dp1-pp1-cp1-tp1"};
    const std::vector<Position> EXPECTED_POSITION = {
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, // y = 0
        {0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}, {7, 1}, // y = 1
        };
    const std::vector<std::vector<uint32_t>> EXPECT_RANKS = {
        {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10}, {11}, {12}, {13}, {14}, {15}
    };
    std::string err;
    algorithm.UpdateParallelDimension(dimension, config, err);
    algorithm.GenerateArrangementByDimension(err);
    ArrangementAndConnectionData data = algorithm.GetArrangementData();
    ASSERT_EQ(data.size, EXPECTED_NAME.size());
    for (const auto& item : data.arrangements) {
        EXPECT_EQ(item.name, EXPECTED_NAME[item.index]);
        EXPECT_EQ(item.position, EXPECTED_POSITION[item.index]);
        EXPECT_EQ(item.ranks.size(), 1);
        EXPECT_EQ(item.ranks.at(0), EXPECT_RANKS[item.index].at(0));
    }
}

TEST_F(MegatronParallelStrategyAlgorithmTest, GetArrangementByDimension_ShouldGetArrangement_TestWithCpDimension)
{
    MegatronParallelStrategyAlgorithm algorithm;
    std::string dimension = DIMENSIONS_CP;
    ParallelStrategyConfig config;
    config.ppSize = 2; // 2
    config.tpSize = 2; // 2
    config.dpSize = 2; // 2
    config.cpSize = 2; // 2
    config.epSize = 2; // 2
    config.algorithm = MEGATRON_LM_TP_CP_EP_DP_PP_ALG;
    const std::vector<std::string> EXPECTED_NAME = {
        "dp0-pp0-cp0", "dp0-pp0-cp1", "dp1-pp0-cp0", "dp1-pp0-cp1",
        "dp0-pp1-cp0", "dp0-pp1-cp1", "dp1-pp1-cp0", "dp1-pp1-cp1"};
    const std::vector<Position> EXPECTED_POSITION = {
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {0, 1}, {1, 1}, {2, 1}, {3, 1}}; // position(x, y)
    const std::vector<std::vector<uint32_t>> EXPECT_RANKS = {
        {0, 1}, {2, 3}, {4, 5}, {6, 7}, {8, 9}, {10, 11}, {12, 13}, {14, 15}
    };
    std::string err;
    algorithm.UpdateParallelDimension(dimension, config, err);
    algorithm.GenerateArrangementByDimension(err);
    ArrangementAndConnectionData data = algorithm.GetArrangementData();
    ASSERT_EQ(data.arrangements.size(), EXPECTED_NAME.size());
    for (const auto& item : data.arrangements) {
        EXPECT_EQ(item.name, EXPECTED_NAME[item.index]);
        EXPECT_EQ(item.position, EXPECTED_POSITION[item.index]);
        EXPECT_EQ(item.ranks.size(), config.tpSize);
        for (int64_t i = 0; i < config.tpSize; ++i) {
            EXPECT_EQ(item.ranks.at(i), EXPECT_RANKS[item.index].at(i));
        }
    }
    config.algorithm = MEGATRON_LM_TP_CP_PP_EP_DP_ALG;
    const std::vector<std::string> EXPECTED_NAME2 = {
        "dp0-pp0-cp0", "dp0-pp0-cp1", "dp0-pp1-cp0", "dp0-pp1-cp1",
        "dp1-pp0-cp0", "dp1-pp0-cp1", "dp1-pp1-cp0", "dp1-pp1-cp1"};

    const std::vector<Position> EXPECTED_POSITION2 = {
        {0, 0}, {1, 0}, {0, 1}, {1, 1}, {2, 0}, {3, 0}, {2, 1}, {3, 1}}; // position(x, y)
    MegatronParallelStrategyAlgorithm algorithm2;
    algorithm2.UpdateParallelDimension(dimension, config, err);
    algorithm2.GenerateArrangementByDimension(err);
    data = algorithm2.GetArrangementData();
    ASSERT_EQ(data.arrangements.size(), EXPECTED_NAME2.size());
    for (const auto& item : data.arrangements) {
        EXPECT_EQ(item.name, EXPECTED_NAME2[item.index]);
        EXPECT_EQ(item.position, EXPECTED_POSITION2[item.index]);
        EXPECT_EQ(item.ranks.size(), config.tpSize);
        for (int64_t i = 0; i < config.tpSize; ++i) {
            EXPECT_EQ(item.ranks.at(i), EXPECT_RANKS[item.index].at(i));
        }
    }
}

TEST_F(MegatronParallelStrategyAlgorithmTest, GetArrangementByDimension_ShouldGetArrangement_TestWithPpDimension)
{
    MegatronParallelStrategyAlgorithm algorithm;
    std::string dimension = DIMENSIONS_PP;
    ParallelStrategyConfig config;
    config.ppSize = 2; // 2
    config.tpSize = 2; // 2
    config.dpSize = 2; // 2
    config.cpSize = 2; // 2
    config.epSize = 2; // 2
    config.algorithm = MEGATRON_LM_TP_CP_EP_DP_PP_ALG;
    const std::vector<std::string> EXPECTED_NAME = {
        "dp0-pp0", "dp1-pp0", "dp0-pp1", "dp1-pp1"};
    const std::vector<std::vector<uint32_t>> EXPECT_RANKS = {
        {0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}
    };
    const std::vector<Position> EXPECTED_POSITION = {{0, 0}, {1, 0}, {0, 1}, {1, 1}}; // position(x, y)
    std::string err;
    algorithm.UpdateParallelDimension(dimension, config, err);
    algorithm.GenerateArrangementByDimension(err);
    ArrangementAndConnectionData data = algorithm.GetArrangementData();
    ASSERT_EQ(data.arrangements.size(), EXPECTED_NAME.size());
    for (const auto& item : data.arrangements) {
        EXPECT_EQ(item.name, EXPECTED_NAME[item.index]);
        EXPECT_EQ(item.position, EXPECTED_POSITION[item.index]);
        EXPECT_EQ(item.ranks.size(), config.tpSize * config.ppSize);
        for (size_t i = 0; i < item.ranks.size(); ++i) {
            EXPECT_EQ(item.ranks.at(i), EXPECT_RANKS[item.index].at(i));
        }
    }
}

TEST_F(MegatronParallelStrategyAlgorithmTest, GetPerformanceByDimension_ShouldReturnFalse_WhenWrongParams)
{
    MegatronParallelStrategyAlgorithm algorithm;
    std::string dimension = DIMENSIONS_TP;
    ParallelStrategyConfig config;
    config.ppSize = 2; // 2
    config.tpSize = 2; // 2
    config.dpSize = 2; // 2
    config.cpSize = 2; // 2
    config.epSize = 2; // 2
    config.algorithm = MEGATRON_LM_TP_CP_EP_DP_PP_ALG;
    std::string err;
    algorithm.UpdateParallelDimension(dimension, config, err);
    GetPerformanceIndicatorParam params;
    params.config = config;
    params.config.algorithm = "xxx";
    std::unordered_map<std::uint32_t, StepStatistic> statistic;
    std::vector<IndicatorDataStruct> responseData;
    bool res = algorithm.GetPerformanceIndicatorByDimension(params, statistic, responseData, err);
    EXPECT_EQ(res, false);
    EXPECT_EQ(err, "Failed to get parallelism performance indicator by dimension. Unexpected parallel config.");
    // wrong dimension
    params.config.algorithm = config.algorithm;
    params.dimension = "xxx";
    res = algorithm.GetPerformanceIndicatorByDimension(params, statistic, responseData, err);
    EXPECT_EQ(res, false);
    EXPECT_EQ(err, "Failed to get parallelism performance indicator by dimension. Unexpected dimension.");
}

void PrepareParametersForGetPerformanceByDimensionTest(ParallelStrategyConfig& config,
    std::unordered_map<std::uint32_t, StepStatistic>& statistic)
{
    config.ppSize = 2; // 2
    config.tpSize = 2; // 2
    config.dpSize = 2; // 2
    config.cpSize = 2; // 2
    config.epSize = 2; // 2
    config.algorithm = MEGATRON_LM_TP_CP_EP_DP_PP_ALG;
    StepStatistic statisticOne;
    statisticOne.computingTime = 90; // 90
    statisticOne.pureCommunicationTime = 60; // 60
    statisticOne.overlapCommunicationTime = 20; // 20
    statisticOne.communicationTime = 80; // 80
    statisticOne.freeTime = 50; // 50
    statisticOne.prepareTime = 10; // 10
    statisticOne.pureCommunicationExcludeReceiveTime = 40; // 40
    uint32_t wordSize = config.ppSize * config.tpSize * config.dpSize * config.cpSize; // 16
    for (uint32_t i = 0; i < wordSize; i++) {
        if (i == 4 || i == 5) { // 4 5 空数据卡
            continue;
        }
        statistic[i] = statisticOne;
    }
}

TEST_F(MegatronParallelStrategyAlgorithmTest, GetPerformanceByDimension_ShouldReturnTrue_TestWithTpDimension)
{
    MegatronParallelStrategyAlgorithm algorithm;
    std::string dimension = DIMENSIONS_TP;
    ParallelStrategyConfig config;
    std::unordered_map<std::uint32_t, StepStatistic> statistic;
    PrepareParametersForGetPerformanceByDimensionTest(config, statistic);
    std::string err;
    algorithm.UpdateParallelDimension(dimension, config, err);
    GetPerformanceIndicatorParam params;
    params.config = config;
    params.dimension = dimension;
    std::vector<IndicatorDataStruct> responseData;
    bool res = algorithm.GetPerformanceIndicatorByDimension(params, statistic, responseData, err);
    EXPECT_EQ(res, true);
    EXPECT_EQ(responseData.size(), 14); // 14 = 16 - 2(empty rank)
}

TEST_F(MegatronParallelStrategyAlgorithmTest, GetPerformanceByDimension_ShouldReturnTrue_TestWithCpDimension)
{
    MegatronParallelStrategyAlgorithm algorithm;
    std::string dimension = DIMENSIONS_CP;
    ParallelStrategyConfig config;
    std::unordered_map<std::uint32_t, StepStatistic> statistic;
    PrepareParametersForGetPerformanceByDimensionTest(config, statistic);
    std::string err;
    algorithm.UpdateParallelDimension(dimension, config, err);
    GetPerformanceIndicatorParam params;
    params.config = config;
    params.dimension = dimension;
    std::vector<IndicatorDataStruct> responseData;
    bool res = algorithm.GetPerformanceIndicatorByDimension(params, statistic, responseData, err);
    EXPECT_EQ(res, true);
    EXPECT_EQ(responseData.size(), 7); // 7 pp groups
}

TEST_F(MegatronParallelStrategyAlgorithmTest, GetPerformanceByDimension_ShouldReturnTrue_TestWithPpDimension)
{
    MegatronParallelStrategyAlgorithm algorithm;
    std::string dimension = DIMENSIONS_PP;
    ParallelStrategyConfig config;
    std::unordered_map<std::uint32_t, StepStatistic> statistic;
    PrepareParametersForGetPerformanceByDimensionTest(config, statistic);
    std::string err;
    algorithm.UpdateParallelDimension(dimension, config, err);
    GetPerformanceIndicatorParam params;
    params.config = config;
    params.dimension = dimension;
    std::vector<IndicatorDataStruct> responseData;
    bool res = algorithm.GetPerformanceIndicatorByDimension(params, statistic, responseData, err);
    EXPECT_EQ(res, true);
    EXPECT_EQ(responseData.size(), 4); // 4 pp groups
    const std::vector<double> EXPECTED_COMPUTING = {90, 90, 90, 90};
    uint32_t i = 0;
    for (auto& item : responseData) {
        EXPECT_EQ(item.indicators[KEY_TOTAL_COMPUTING_TIME + KEY_MAX_SUFFIX], EXPECTED_COMPUTING[i++]);
    }
    //  test for MEGATRON_LM_TP_CP_PP_EP_DP_ALG
    config.algorithm = MEGATRON_LM_TP_CP_PP_EP_DP_ALG;
    algorithm.ClearStrategyConfigCache();
    algorithm.UpdateParallelDimension(dimension, config, err);
    params.config = config;
    responseData.clear();
    res = algorithm.GetPerformanceIndicatorByDimension(params, statistic, responseData, err);
    EXPECT_EQ(res, true);
    EXPECT_EQ(responseData.size(), 4); // 4 cp groups
    const std::vector<double> EXPECTED_COMPUTING2 = {90, 90, 90, 90};
    i = 0;
    for (auto& item : responseData) {
        EXPECT_EQ(item.indicators[KEY_TOTAL_COMPUTING_TIME + KEY_MAX_SUFFIX], EXPECTED_COMPUTING2[i++]);
    }
}

TEST_F(MegatronParallelStrategyAlgorithmTest, GetPerformanceByDimension_ShouldReturnTrue_TestWithDpDimension)
{
    MegatronParallelStrategyAlgorithm algorithm;
    std::string dimension = DIMENSIONS_DP;
    ParallelStrategyConfig config;
    std::unordered_map<std::uint32_t, StepStatistic> statistic;
    PrepareParametersForGetPerformanceByDimensionTest(config, statistic);
    std::string err;
    algorithm.UpdateParallelDimension(dimension, config, err);
    GetPerformanceIndicatorParam params;
    params.config = config;
    params.dimension = dimension;
    std::vector<IndicatorDataStruct> responseData;
    bool res = algorithm.GetPerformanceIndicatorByDimension(params, statistic, responseData, err);
    EXPECT_EQ(res, true);
    EXPECT_EQ(responseData.size(), 2); // 2 dp groups
    const std::vector<double> EXPECTED_COMPUTING = {180, 180};
    uint32_t i = 0;
    for (auto& item : responseData) {
        EXPECT_EQ(item.indicators[VALUE_SUM_OF_MAX + KEY_TOTAL_COMPUTING_TIME], EXPECTED_COMPUTING[i++]);
    }
}