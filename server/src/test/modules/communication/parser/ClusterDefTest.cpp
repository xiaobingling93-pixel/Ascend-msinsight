/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "ClusterDef.h"
class ClusterDefTest : public ::testing::Test {};

/**
 * 测试ParallelStrategyConfig
 */
using namespace Dic::Module;
TEST_F(ClusterDefTest, TestParallelStrategyConfigForBaseCheck)
{
    const int64_t maxSize = Dic::Module::MAX_PARALLEL_SIZE + 1;
    const int64_t minSize = 1;
    ParallelStrategyConfig config1 = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, maxSize, minSize, minSize, minSize, minSize,
                                       minSize };
    ParallelStrategyConfig config2 = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, minSize, maxSize, minSize, minSize, minSize,
                                       minSize };
    ParallelStrategyConfig config3 = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, minSize, minSize, maxSize, minSize, minSize,
                                       minSize };
    ParallelStrategyConfig config4 = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, minSize, minSize, minSize, maxSize, minSize,
                                       minSize };
    ParallelStrategyConfig config5 = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, minSize, minSize, minSize, minSize, maxSize,
                                       minSize };
    ParallelStrategyConfig config11 = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, minSize, minSize, minSize, minSize, minSize,
                                        maxSize };
    ParallelStrategyConfig config6 = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, 0, minSize, minSize, minSize, minSize, minSize };
    ParallelStrategyConfig config7 = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, minSize, 0, minSize, minSize, minSize, minSize };
    ParallelStrategyConfig config8 = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, minSize, minSize, 0, minSize, minSize, minSize };
    ParallelStrategyConfig config9 = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, minSize, minSize, minSize, 0, minSize, minSize };
    ParallelStrategyConfig config10 = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, minSize, minSize, minSize, minSize, 0, minSize};
    ParallelStrategyConfig config12 = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, minSize, minSize, minSize, minSize, minSize, 0};
    std::string error;
    EXPECT_EQ(config1.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] PP size must be between 1 and "+ std::to_string(MAX_PARALLEL_SIZE));
    EXPECT_EQ(config2.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] TP size must be between 1 and "+ std::to_string(MAX_PARALLEL_SIZE));
    EXPECT_EQ(config3.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] DP size must be between 1 and "+ std::to_string(MAX_PARALLEL_SIZE));
    EXPECT_EQ(config4.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] CP size must be between 1 and "+ std::to_string(MAX_PARALLEL_SIZE));
    EXPECT_EQ(config5.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] EP size must be between 1 and "+ std::to_string(MAX_PARALLEL_SIZE));
    EXPECT_EQ(config11.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] MOE_TP size must be between 1 and "+ std::to_string(MAX_PARALLEL_SIZE));
    EXPECT_EQ(config6.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] PP size must be between 1 and "+ std::to_string(MAX_PARALLEL_SIZE));
    EXPECT_EQ(config7.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] TP size must be between 1 and "+ std::to_string(MAX_PARALLEL_SIZE));
    EXPECT_EQ(config8.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] DP size must be between 1 and "+ std::to_string(MAX_PARALLEL_SIZE));
    EXPECT_EQ(config9.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] CP size must be between 1 and "+ std::to_string(MAX_PARALLEL_SIZE));
    EXPECT_EQ(config10.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] EP size must be between 1 and "+ std::to_string(MAX_PARALLEL_SIZE));
    EXPECT_EQ(config12.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] MOE_TP size must be between 1 and "+ std::to_string(MAX_PARALLEL_SIZE));
}

// Test for DP size evenly divided by EP size
TEST_F(ClusterDefTest, TestDPSizeEvenlyDividedByEPSize)
{
    ParallelStrategyConfig config = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, 5, 5, 100, 100, 254 };
    std::string error;
    EXPECT_EQ(config.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] DP size must be evenly divided by EP Size.");
}

// Test for product of PP, TP, DP, CP sizes being less than MAX_WORLD_SIZE
TEST_F(ClusterDefTest, TestProductOfPPAndTPAndDPAndCPSizesLessThanUINT32Max)
{
    ParallelStrategyConfig config = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, 10000, 10000, 10000, 10000, 100 };
    std::string error;
    EXPECT_EQ(config.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] The product of PP size, TP size, DP size, and CP size must be less than " +
        std::to_string(MAX_WORLD_SIZE));
}

// Test for DP and CP sizes being evenly divided by EP size
TEST_F(ClusterDefTest, TestDPAndCPSizeEvenlyDividedByEPSize)
{
    ParallelStrategyConfig config = { MINDSPEED_TP_CP_EP_DP_PP_ALG, 5, 5, 10, 10, 254 };
    std::string error;
    EXPECT_EQ(config.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] The product of DP size and CP size must be evenly divided by EP Size.");
}

// Test for MOE algorithm validation (MOE_TP and EP sizes match TP and DP sizes)
TEST_F(ClusterDefTest, TestMOEAlgorithmValidation)
{
    ParallelStrategyConfig config = { MINDIE_LLM_TP_DP_EP_PP_MOETP_ALG, 5, 5, 5, 1, 5, 1 };
    std::string error;
    EXPECT_EQ(config.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] The product of MOE_TP size and EP size should match the product of TP size and "
                     "DP size for the MindIE-LLM.");
}

// Test for unsupported CP parallelism in MOE algorithm
TEST_F(ClusterDefTest, TestUnsupportedCPParallelismForMOEAlgorithm)
{
    ParallelStrategyConfig config = { MINDIE_LLM_TP_DP_EP_PP_MOETP_ALG, 5, 5, 5, 5, 5, 5 };
    std::string error;
    EXPECT_EQ(config.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] The CP Parallelism is not supported by the MOE algorithm.");
}

// Test for DP and TP sizes being evenly divided by EP size
TEST_F(ClusterDefTest, TestDPAndTPSizeEvenlyDividedByEPSize)
{
    ParallelStrategyConfig config = { VLLM_TP_PP_DP_EP_ALG, 5, 5, 5, 1, 2, 1 };
    std::string error;
    EXPECT_EQ(config.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] The product of DP size and TP size must be evenly divided by EP Size.");
}

// Test for EP size being evenly divided by TP size
TEST_F(ClusterDefTest, TestEPSizeEvenlyDividedByTPSize)
{
    ParallelStrategyConfig config = { VLLM_TP_PP_DP_EP_ALG, 2, 8, 2, 1, 2, 1 };
    std::string error;
    EXPECT_EQ(config.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] EP size must be evenly divided by TP Size.");
}

// Test for valid configuration (all parameters are valid)
TEST_F(ClusterDefTest, TestValidConfiguration)
{
    std::string error = "";
    ParallelStrategyConfig config = { MEGATRON_LM_TP_CP_EP_DP_PP_ALG, 2, 2, 4, 2, 2 };
    EXPECT_EQ(config.CheckParams(error), true);
    EXPECT_EQ(error, "");
}
