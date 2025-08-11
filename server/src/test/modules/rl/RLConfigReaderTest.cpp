/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "RLMstxConfigReader.h"
#include "FileUtil.h"
#include "gtest/gtest.h"

using namespace Dic::Module::RL;
using namespace Dic;

class RLConfigReaderTest : public testing::Test {
};

TEST_F(RLConfigReaderTest, emptyConfigPath)
{
    RLMstxConfigReader reader;
    reader.SetConfigPath("");
    EXPECT_TRUE(reader.ReadConfigFile().empty());
}

TEST_F(RLConfigReaderTest, InvalidPath)
{
    RLMstxConfigReader reader;
    reader.SetConfigPath("/home/xx/test_no_exist_dir");
    EXPECT_TRUE(reader.ReadConfigFile().empty());
}

TEST_F(RLConfigReaderTest, Normal)
{
    std::string currentPath = FileUtil::GetCurrPath();
    auto pos = currentPath.find("server");
    std::string srcPath = currentPath.substr(0, pos);
    std::string configPath = FileUtil::SplicePath(srcPath, "server", "src", "test", "test_data", "rl", "RLConfig.json");
    RLMstxConfigReader reader;
    reader.SetConfigPath(configPath);
    auto res = reader.ReadConfigFile();
    EXPECT_EQ(res.size(), 2); // expect 2 config
    auto &config1 = res[0];
    EXPECT_EQ(config1.algorithm, "grpo");
    EXPECT_EQ(config1.framework, "verl");
    EXPECT_EQ(config1.taskConfigs.size(), 2); // expect contains 2 stage
    EXPECT_EQ(config1.taskConfigs[0].roleName, "actor");
    EXPECT_EQ(config1.taskConfigs[0].taskName, "generator_sequence");
    EXPECT_EQ(config1.taskConfigs[0].microBatchConfigs.size(), 2); // stage contains 2 microbatch
    EXPECT_EQ(config1.taskConfigs[0].microBatchConfigs[0].batchName, "actor_micro_batch1");
    EXPECT_EQ(config1.taskConfigs[0].microBatchConfigs[0].type, "FP");
}