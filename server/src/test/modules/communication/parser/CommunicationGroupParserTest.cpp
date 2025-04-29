/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "FileUtil.h"
#include "CommunicationGroupParser.h"
#include "ClusterDef.h"

using namespace Dic::Module;
using namespace Dic::Module::Communication;
class CommunicationGroupParserTest : public ::testing::Test {
protected:
    std::string filePath;

    void SetUp() override
    {
        std::string  currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        filePath = currPath + R"(/src/test/test_data/communication_group/)";
    }
};

/**
 * 测试新的communication_grou.json格式（带有comm_group_parallel_info数据）
 */
TEST_F(CommunicationGroupParserTest, TestNewCommunicationGroup)
{
    std::string file = filePath + R"(communication_group.json)";
    std::vector<CommGroupParallelInfo> res = CommunicationGroupParser::ParseCommunicationGroup(file);
    const int exceptSize = 22;
    EXPECT_EQ(res.size(), exceptSize);
}

/**
 * 测试老的communication_grou.json格式（不带comm_group_parallel_info数据）
 */
TEST_F(CommunicationGroupParserTest, TestOldCommunicationGroup)
{
    std::string file = filePath + R"(communication_group_old.json)";
    std::vector<CommGroupParallelInfo> res = CommunicationGroupParser::ParseCommunicationGroup(file);
    const int exceptSize = 20;
    EXPECT_EQ(res.size(), exceptSize);
}