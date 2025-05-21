/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "IEContext.h"
#include "FileUtil.h"
#include "ServitizationContext.h"
using namespace Dic::Module::IE;
class ServitizationContextTest : public ::testing::Test {
    void SetUp() override
    {
        IEContext::Instance().Reset();
    }

    void TearDown() override
    {
        IEContext::Instance().Reset();
    }
};

TEST_F(ServitizationContextTest, InitDataBaseTest)
{
    std::shared_ptr<ServitizationContext> context = std::make_shared<ServitizationContext>();
    auto res = context->InitDataBase("llllllll", ":memory:");
    EXPECT_EQ(res, true);
    auto res2 = context->InitDataBase("llllllll", ":memory:");
    EXPECT_EQ(res2, false);
}

TEST_F(ServitizationContextTest, ComputeFileIdByFolderTest)
{
    std::shared_ptr<ServitizationContext> context = std::make_shared<ServitizationContext>();
    std::string folder = R"(/src/test/test_data)";
    auto res = context->ComputeFileIdByFolder(folder);
    EXPECT_EQ(res, "test_data_0");
    auto res2 = context->ComputeFileIdByFolder(folder);
    EXPECT_EQ(res2, "test_data_0");
}

TEST_F(ServitizationContextTest, GetDatabaseTest)
{
    std::shared_ptr<ServitizationContext> context = std::make_shared<ServitizationContext>();
    auto res = context->GetDatabase("kk");
    EXPECT_EQ(res, nullptr);
}

TEST_F(ServitizationContextTest, ExecuteScriptTest)
{
    std::shared_ptr<ServitizationContext> context = std::make_shared<ServitizationContext>();
    context->Reset();
}