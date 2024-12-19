/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MegatronParallelStrategyAlgorithm.h"
using namespace Dic::Module;
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
    dimension = DIMENSIONS_PP;
    res = algorithm3.UpdateParallelDimension(dimension, config, err);
    EXPECT_EQ(algorithm3.GetArrangementData().size, 16); // 16 = 2*2*4
    EXPECT_TRUE(res);
    MegatronParallelStrategyAlgorithm algorithm4;
    dimension = DIMENSIONS_CP;
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
        "dp0-cp0-pp0-tp0", "dp0-cp0-pp0-tp1", "dp0-cp1-pp0-tp0", "dp0-cp1-pp0-tp1",
        "dp1-cp0-pp0-tp0", "dp1-cp0-pp0-tp1", "dp1-cp1-pp0-tp0", "dp1-cp1-pp0-tp1",
        "dp0-cp0-pp1-tp0", "dp0-cp0-pp1-tp1", "dp0-cp1-pp1-tp0", "dp0-cp1-pp1-tp1",
        "dp1-cp0-pp1-tp0", "dp1-cp0-pp1-tp1", "dp1-cp1-pp1-tp0", "dp1-cp1-pp1-tp1"};
    const std::vector<Position> EXPECTED_POSITION = {
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, // y = 0
        {0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}, {7, 1}, // y = 1
        };
    std::string err;
    algorithm.UpdateParallelDimension(dimension, config, err);
    algorithm.GenerateArrangementByDimension();
    ArrangementAndConnectionData data = algorithm.GetArrangementData();
    ASSERT_EQ(data.size, EXPECTED_NAME.size());
    for (const auto& item : data.arrangements) {
        EXPECT_EQ(item.name, EXPECTED_NAME[item.index]);
        EXPECT_EQ(item.position, EXPECTED_POSITION[item.index]);
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
        "dp0-cp0-pp0", "dp0-cp1-pp0", "dp1-cp0-pp0", "dp1-cp1-pp0",
        "dp0-cp0-pp1", "dp0-cp1-pp1", "dp1-cp0-pp1", "dp1-cp1-pp1"};
    const std::vector<Position> EXPECTED_POSITION = {
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {0, 1}, {1, 1}, {2, 1}, {3, 1}}; // position(x, y)
    std::string err;
    algorithm.UpdateParallelDimension(dimension, config, err);
    algorithm.GenerateArrangementByDimension();
    ArrangementAndConnectionData data = algorithm.GetArrangementData();
    ASSERT_EQ(data.arrangements.size(), EXPECTED_NAME.size());
    for (const auto& item : data.arrangements) {
        EXPECT_EQ(item.name, EXPECTED_NAME[item.index]);
        EXPECT_EQ(item.position, EXPECTED_POSITION[item.index]);
    }
    config.algorithm = MEGATRON_LM_TP_CP_PP_EP_DP_ALG;
    const std::vector<std::string> EXPECTED_NAME2 = {
        "dp0-cp0-pp0", "dp0-cp1-pp0", "dp0-cp0-pp1", "dp0-cp1-pp1",
        "dp1-cp0-pp0", "dp1-cp1-pp0", "dp1-cp0-pp1", "dp1-cp1-pp1"};

    const std::vector<Position> EXPECTED_POSITION2 = {
        {0, 0}, {1, 0}, {0, 1}, {1, 1}, {2, 0}, {3, 0}, {2, 1}, {3, 1}}; // position(x, y)
    MegatronParallelStrategyAlgorithm algorithm2;
    algorithm2.UpdateParallelDimension(dimension, config, err);
    algorithm2.GenerateArrangementByDimension();
    data = algorithm2.GetArrangementData();
    ASSERT_EQ(data.arrangements.size(), EXPECTED_NAME2.size());
    for (const auto& item : data.arrangements) {
        EXPECT_EQ(item.name, EXPECTED_NAME2[item.index]);
        EXPECT_EQ(item.position, EXPECTED_POSITION2[item.index]);
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
        "dp0-cp0", "dp0-cp1", "dp1-cp0", "dp1-cp1"};
    const std::vector<Position> EXPECTED_POSITION = {{0, 0}, {1, 0}, {2, 0}, {3, 0}}; // position(x, y)
    std::string err;
    algorithm.UpdateParallelDimension(dimension, config, err);
    algorithm.GenerateArrangementByDimension();
    ArrangementAndConnectionData data = algorithm.GetArrangementData();
    ASSERT_EQ(data.arrangements.size(), EXPECTED_NAME.size());
    for (const auto& item : data.arrangements) {
        EXPECT_EQ(item.name, EXPECTED_NAME[item.index]);
        EXPECT_EQ(item.position, EXPECTED_POSITION[item.index]);
    }
}