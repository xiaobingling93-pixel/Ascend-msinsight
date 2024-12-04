/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "FileUtil.h"
#include "JupyterFileParser.h"
#include "JupyterServerManager.h"

using namespace Dic::Module::Jupyter;
class JupyterInstanceTest : public ::testing::Test {
};

TEST_F(JupyterInstanceTest, GetThreadPoolForJupyterFileParserTest)
{
    auto threadpool = JupyterFileParser::Instance().GetThreadPool();
    EXPECT_NE(threadpool, nullptr);
}

TEST_F(JupyterInstanceTest, JupyterServerManagerStartTestReturnFalseWhenInvalidPath)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    bool result = JupyterServerManager::Instance().Start(Dic::FileUtil::SplicePath(currPath, "jupyter&"));
    EXPECT_EQ(result, false);
}