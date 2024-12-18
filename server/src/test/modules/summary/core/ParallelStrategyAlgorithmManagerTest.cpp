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
    bool UpdateParallelDimension(const std::string &dimension,
                                 const ParallelStrategyConfig &tmpConfig, std::string &err) override { return true; }
    void GenerateArrangementByDimension() override {}
    ArrangementAndConnectionData GetArrangementData() override { return data; }
private:
    ArrangementAndConnectionData data;
};

TEST_F(ParallelStrategyAlgorithmManagerTest, AddAlgorithm_ShouldReturnFalse_WhenProjectAlreadyExists)
{
    std::string projectName = "testProject";
    auto algPtr = std::make_shared<MockParallelStrategyAlgorithm>();
    ParallelStrategyAlgorithmManager::Instance().AddAlgorithm(projectName, algPtr);
    bool result = ParallelStrategyAlgorithmManager::Instance().AddAlgorithm(projectName, algPtr);
    EXPECT_FALSE(result);
}

TEST_F(ParallelStrategyAlgorithmManagerTest, AddAlgorithm_ShouldReturnTrue_WhenProjectNotExists)
{
    std::string projectName = "testProject";
    auto algPtr = std::make_shared<MockParallelStrategyAlgorithm>();
    bool result = ParallelStrategyAlgorithmManager::Instance().AddAlgorithm(projectName, algPtr);
    EXPECT_TRUE(result);
}

TEST_F(ParallelStrategyAlgorithmManagerTest, DeleteAlgorithm_ShouldReturnFalse_WhenProjectNotExist)
{
    std::string projectName = "NonExistentProject";
    bool result = ParallelStrategyAlgorithmManager::Instance().DeleteAlgorithm(projectName);
    EXPECT_FALSE(result);
}

TEST_F(ParallelStrategyAlgorithmManagerTest, DeleteAlgorithm_ShouldReturnTrue_WhenProjectExist)
{
    std::string projectName = "ExistentProject";
    auto algPtr = std::make_shared<MockParallelStrategyAlgorithm>();
    ParallelStrategyAlgorithmManager::Instance().AddAlgorithm(projectName, algPtr);
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
    ParallelStrategyAlgorithmManager::Instance().AddAlgorithm(projectName, algPtr);
    std::shared_ptr<BaseParallelStrategyAlgorithm> result =
        ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName("ExistentProject", err);
    EXPECT_NE(result, nullptr);
}
