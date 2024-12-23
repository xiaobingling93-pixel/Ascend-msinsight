/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include <memory>
#include "ParallelStrategyAlgorithmManager.h"
using namespace Dic::Module;
class ParallelStrategyAlgorithmManagerTest : public ::testing::Test {
public:
    void SetUp() override
    {
        ParallelStrategyAlgorithmManager::Instance().Reset();
    }

    void TearDown() override
    {
        ParallelStrategyAlgorithmManager::Instance().Reset();
    }
};

class MockParallelStrategyAlgorithm : public BaseParallelStrategyAlgorithm {
public:
    void ClearStrategyConfigCache() override {};
    bool UpdateParallelDimension(const std::string &dimension,
                                 const ParallelStrategyConfig &tmpConfig, std::string &err) override { return true; }
    void GenerateArrangementByDimension() override {}
    ArrangementAndConnectionData GetArrangementData() override { return data; }
    bool GetPerformanceIndicatorByDimension(const Dic::Protocol::ParallelismPerformance &config,
        const std::unordered_map<std::uint32_t, StepStatistic> &statistic, PerformanceIndicatorData &performanceData,
        std::string &err) override
    {
        return true;
    }

private:
    ArrangementAndConnectionData data;
};

TEST_F(ParallelStrategyAlgorithmManagerTest, AddAlgorithm_ShouldReturnFalse_WhenProjectAlreadyExists)
{
    std::string projectName = "testProject";
    ParallelStrategyConfig config;
    auto algPtr = std::make_shared<MockParallelStrategyAlgorithm>();
    ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(projectName, algPtr, config);
    ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(projectName, algPtr, config);
}

TEST_F(ParallelStrategyAlgorithmManagerTest, AddAlgorithm_ShouldReturnTrue_WhenProjectNotExists)
{
    std::string projectName = "testProject";
    ParallelStrategyConfig config;
    auto algPtr = std::make_shared<MockParallelStrategyAlgorithm>();
    ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(projectName, algPtr, config);
}

TEST_F(ParallelStrategyAlgorithmManagerTest, DeleteAlgorithm_ShouldReturnFalse_WhenProjectNotExist)
{
    std::string projectName = "NonExistentProject";
    ParallelStrategyConfig config;
    bool result = ParallelStrategyAlgorithmManager::Instance().DeleteAlgorithm(projectName);
    EXPECT_FALSE(result);
}

TEST_F(ParallelStrategyAlgorithmManagerTest, DeleteAlgorithm_ShouldReturnTrue_WhenProjectExist)
{
    std::string projectName = "ExistentProject";
    ParallelStrategyConfig config;
    auto algPtr = std::make_shared<MockParallelStrategyAlgorithm>();
    ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(projectName, algPtr, config);
    bool result = ParallelStrategyAlgorithmManager::Instance().DeleteAlgorithm(projectName);
    EXPECT_TRUE(result);
}

TEST_F(ParallelStrategyAlgorithmManagerTest, GetAlgorithmByProjectName_ShouldReturnNull_WhenProjectNameNotExist)
{
    std::string err;
    std::shared_ptr<BaseParallelStrategyAlgorithm> result =
        ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName("NonExistentProject", err);
    EXPECT_EQ(result, nullptr);
}

TEST_F(ParallelStrategyAlgorithmManagerTest, GetAlgorithmByProjectName_ShouldReturnAlgorithm_WhenProjectNameExist)
{
    std::string err;
    std::shared_ptr<BaseParallelStrategyAlgorithm> algPtr = std::make_shared<MockParallelStrategyAlgorithm>();
    std::string projectName = "ExistentProject";
    ParallelStrategyConfig config;
    ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(projectName, algPtr, config);
    std::shared_ptr<BaseParallelStrategyAlgorithm> result =
        ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName("ExistentProject", err);
    EXPECT_NE(result, nullptr);
}
