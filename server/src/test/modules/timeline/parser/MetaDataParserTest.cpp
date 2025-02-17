/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MetaDataParser.h"
#include "FileUtil.h"

using namespace Dic::Module::Timeline;
class MetaDataParserTest : public ::testing::Test {
protected:
    std::string filePath;
    void SetUp() override
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        filePath = currPath + R"(/src/test/test_data/metadata/profiler_metadata.json)";
    }
};

/**
 * 测试解析profiler_metadata文件成功
 */
TEST_F(MetaDataParserTest, test_ParserParallelGroupInfoByFilePath_success)
{
    std::vector<ParallelGroupInfo> res = MetaDataParser::ParserParallelGroupInfoByFilePath(filePath);
    const int twentyOne = 21;
    EXPECT_EQ(res.size(), twentyOne);
}

/**
 * 测试解析parallel_group_info数据失败，文本内容为空
 */
TEST_F(MetaDataParserTest, test_ParserParallelGroupInfoByText_fail_with_empty_text)
{
    std::vector<ParallelGroupInfo> res = MetaDataParser::ParserParallelGroupInfoByText("");
    const int zero = 0;
    EXPECT_EQ(res.size(), zero);
}

/**
 * 测试解析parallel_group_info数据成功
 */
TEST_F(MetaDataParserTest, test_ParserParallelGroupInfoByText_success)
{
    std::string text = "{\"10.174.216.241%enp189s0f1_55000_0_1738895486152654\": {\"group_name\": \"default_group\", "
                       "\"group_rank\": 0, \"global_ranks\": [0, 1, 2, 3, 4, 5, 6, 7]}, "
                       "\"10.174.216.241%enp189s0f1_55000_0_1738895521183247\": {\"group_name\": \"dp\", "
                       "\"group_rank\": 0, \"global_ranks\": [0, 2]}, "
                       "\"10.174.216.241%enp189s0f1_55000_0_1738895510668950\": {\"group_name\": \"dp_cp\", "
                       "\"group_rank\": 0, \"global_ranks\": [0, 2]}, "
                       "\"10.174.216.241%enp189s0f1_55000_0_1738895521237312\": {\"group_name\": \"cp\", "
                       "\"group_rank\": 0, \"global_ranks\": [0]}, "
                       "\"10.174.216.241%enp189s0f1_55000_0_1738895511513392\": {\"group_name\": \"mp\", "
                       "\"group_rank\": 0, \"global_ranks\": [0, 1, 4, 5]}, "
                       "\"10.174.216.241%enp189s0f1_55000_0_1738895521288624\": {\"group_name\": "
                       "\"mp_exp\", \"group_rank\": 0, \"global_ranks\": [0, 1, 4, 5]}, "
                       "\"10.174.216.241%enp189s0f1_55000_0_1738895506630464\": "
                       "{\"group_name\": \"tp\", \"group_rank\": 0, \"global_ranks\": [0, 1]}}";
    std::vector<ParallelGroupInfo> res = MetaDataParser::ParserParallelGroupInfoByText(text);
    const int seven = 7;
    EXPECT_EQ(res.size(), seven);
}
