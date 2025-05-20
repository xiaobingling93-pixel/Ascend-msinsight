/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "VLLMParallelStrategyAlgorithm.h"
#include "ParallelStrategyAlgorithmManager.h"
using namespace Dic::Module;
using namespace Dic::Protocol;
using namespace Dic::Module::Summary;
class VLLMParallelStrategyAlgorithmTest : public ::testing::Test {
};

TEST_F(VLLMParallelStrategyAlgorithmTest, UpdateParallelDimension_ShouldReturnTrue_WhenUpdateSuccess)
{
    VLLMParallelStrategyAlgorithm algorithm;
    std::string dimension = DIMENSIONS_TP;
    ParallelStrategyConfig config;
    config.ppSize = 2; // 2
    config.tpSize = 2; // 2
    config.dpSize = 4; // 4
    config.epSize = 4; // 4
    config.algorithm = VLLM_TP_PP_DP_EP_ALG;
    std::string err;
    bool res = algorithm.UpdateParallelDimension(dimension, config, err);
    EXPECT_TRUE(res);
}

TEST_F(VLLMParallelStrategyAlgorithmTest, UpdateParallelDimension_ShouldReturnFalse_WhenWrongInput)
{
    VLLMParallelStrategyAlgorithm algorithm;
    std::string dimension = "yyyyy";
    ParallelStrategyConfig config;
    config.ppSize = 2; // 2
    config.tpSize = 2; // 2
    config.dpSize = 4; // 4
    config.epSize = 4; // 4
    config.algorithm = "xxxx";
    std::string err;
    bool res = algorithm.UpdateParallelDimension(dimension, config, err);
    EXPECT_FALSE(res);
    EXPECT_EQ(err, "Failed to update parallel view. Unexpected algorithm for the vLLM.");
    VLLMParallelStrategyAlgorithm algorithm2;
    config.algorithm = VLLM_TP_PP_DP_EP_ALG;
    res = algorithm.UpdateParallelDimension(dimension, config, err);
    EXPECT_FALSE(res);
    EXPECT_EQ(err, "Failed to update show map for parallel view. Unexpected dimension.");
}

TEST_F(VLLMParallelStrategyAlgorithmTest, GetArrangementByDimension_ShouldGetArrangement_TestWithPp2Tp2Dp2Ep2)
{
    std::string dimension = DIMENSIONS_TP;
    ParallelStrategyConfig config;
    config.ppSize = 2; // 2
    config.tpSize = 2; // 2
    config.dpSize = 2; // 2
    config.epSize = 2; // 2
    config.algorithm = VLLM_TP_PP_DP_EP_ALG;
    std::string err;
    std::string projectName = "testProject";
    ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(projectName, config, err);
    auto algorithm = ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName("testProject");
    algorithm->UpdateParallelDimension(dimension, config, err);
    algorithm->GenerateArrangementByDimension(err);
    ArrangementAndConnectionData data = algorithm->GetArrangementData();
    const std::vector<std::string> EXPECTED_NAME = {
        "dp0-pp0-tp0", "dp0-pp0-tp1", "dp0-pp1-tp0", "dp0-pp1-tp1",
        "dp1-pp0-tp0", "dp1-pp0-tp1", "dp1-pp1-tp0", "dp1-pp1-tp1"};
    const std::vector<Position> EXPECTED_POSITION = {
        {0, 0}, {1, 0}, {0, 1}, {1, 1},
        {2, 0}, {3, 0}, {2, 1}, {3, 1}
    };
    const std::vector<std::vector<uint32_t>> EXPECT_RANKS = {
        {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}
    };
    ASSERT_EQ(data.size, EXPECTED_NAME.size());
    for (const auto& item : data.arrangements) {
        EXPECT_EQ(item.name, EXPECTED_NAME[item.index]);
        EXPECT_EQ(item.position, EXPECTED_POSITION[item.index]);
        EXPECT_EQ(item.ranks.size(), 1);
        EXPECT_EQ(item.ranks.at(0), EXPECT_RANKS[item.index].at(0));
    }
    // 16个connections: 'pp': 4, 'dp': 4, 'tp': 4, 'exp': 4
    EXPECT_EQ(data.connections.size(), 16); // 16
    const std::vector<std::vector<uint32_t>> EXPECT_INDEXES = {
        {0, 1}, {2, 3}, {4, 5}, {6, 7}
    };
    int i = 0;
    for (const auto& item : data.connections) {
        if (item.type != "exp") {
            continue;
        }
        ASSERT_LE(i, EXPECT_INDEXES.size());
        EXPECT_EQ(item.indexes, EXPECT_INDEXES[i++]);
    }
}

TEST_F(VLLMParallelStrategyAlgorithmTest, GetArrangementByDimension_ShouldGetArrangement_TestWithPp2Tp2Dp2Ep4)
{
    std::string dimension = DIMENSIONS_TP;
    ParallelStrategyConfig config;
    config.ppSize = 2; // 2
    config.tpSize = 2; // 2
    config.dpSize = 2; // 2
    config.epSize = 4; // 4
    config.algorithm = VLLM_TP_PP_DP_EP_ALG;
    std::string err;
    std::string projectName = "testProject";
    ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(projectName, config, err);
    auto algorithm = ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName("testProject");
    algorithm->UpdateParallelDimension(dimension, config, err);
    algorithm->GenerateArrangementByDimension(err);
    ArrangementAndConnectionData data = algorithm->GetArrangementData();
    const std::vector<std::string> EXPECTED_NAME = {
        "dp0-pp0-tp0", "dp0-pp0-tp1", "dp0-pp1-tp0", "dp0-pp1-tp1",
        "dp1-pp0-tp0", "dp1-pp0-tp1", "dp1-pp1-tp0", "dp1-pp1-tp1"};
    const std::vector<Position> EXPECTED_POSITION = {
        {0, 0}, {1, 0}, {0, 1}, {1, 1},
        {2, 0}, {3, 0}, {2, 1}, {3, 1}
    };
    const std::vector<std::vector<uint32_t>> EXPECT_RANKS = {
        {0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}
    };
    ASSERT_EQ(data.size, EXPECTED_NAME.size());
    for (const auto& item : data.arrangements) {
        EXPECT_EQ(item.name, EXPECTED_NAME[item.index]);
        EXPECT_EQ(item.position, EXPECTED_POSITION[item.index]);
        EXPECT_EQ(item.ranks.size(), 1);
        EXPECT_EQ(item.ranks.at(0), EXPECT_RANKS[item.index].at(0));
    }
    // 14个connections: 'pp': 4, 'dp': 4, 'tp': 4, 'exp': 2
    EXPECT_EQ(data.connections.size(), 14); // 14
    const std::vector<std::vector<uint32_t>> EXPECT_INDEXES = {
        {0, 1, 4, 5}, {2, 3, 6, 7}
    };
    int i = 0;
    for (const auto& item : data.connections) {
        if (item.type != "exp") {
            continue;
        }
        ASSERT_LE(i, EXPECT_INDEXES.size());
        EXPECT_EQ(item.indexes, EXPECT_INDEXES[i++]);
    }
}