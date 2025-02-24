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
TEST_F(ClusterDefTest, TestParallelStrategyConfig)
{
    const int64_t maxSize = Dic::Module::MAX_PARALLEL_SIZE + 1;
    const int64_t minSize = 1;
    Dic::Module::ParallelStrategyConfig config1 = { "", maxSize, minSize, minSize, minSize, minSize };
    Dic::Module::ParallelStrategyConfig config2 = { "", minSize, maxSize, minSize, minSize, minSize };
    Dic::Module::ParallelStrategyConfig config3 = { "", minSize, minSize, maxSize, minSize, minSize };
    Dic::Module::ParallelStrategyConfig config4 = { "", minSize, minSize, minSize, maxSize, minSize };
    Dic::Module::ParallelStrategyConfig config5 = { "", minSize, minSize, minSize, minSize, maxSize };
    Dic::Module::ParallelStrategyConfig config6 = { "", 0, minSize, minSize, minSize, minSize };
    Dic::Module::ParallelStrategyConfig config7 = { "", minSize, 0, minSize, minSize, minSize };
    Dic::Module::ParallelStrategyConfig config8 = { "", minSize, minSize, 0, minSize, minSize };
    Dic::Module::ParallelStrategyConfig config9 = { "", minSize, minSize, minSize, 0, minSize };
    Dic::Module::ParallelStrategyConfig config10 = { "", minSize, minSize, minSize, minSize, 0 };
    Dic::Module::ParallelStrategyConfig config11 = { "", 5, 5, 100, 100, 254 };
    Dic::Module::ParallelStrategyConfig config12 = { "", 10000, 10000, 10000, 10000, 100 };
    Dic::Module::ParallelStrategyConfig config13 = { "", 2, 2, 4, 2, 2 };
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
    EXPECT_EQ(config11.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] DP size must be evenly divided by EP Size.");
    EXPECT_EQ(config12.CheckParams(error), false);
    EXPECT_EQ(error, "[Summary] The product of PP size, TP size, DP size, and CP size must be less than " +
        std::to_string(UINT32_MAX));
    error = "";
    EXPECT_EQ(config13.CheckParams(error), true);
    EXPECT_EQ(error, "");
}
