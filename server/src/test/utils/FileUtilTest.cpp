/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "FileUtil.h"
#include "../TestSuit.cpp"
#include "JsonUtil.h"

using namespace Dic;

class FileUtilTest : TestSuit {
};

TEST_F(TestSuit, BasicAssertions)
{
#ifdef _WIN32
    EXPECT_EQ(FileUtil::SplicePath("a", "b"), "a\\b");
#else
    EXPECT_EQ(FileUtil::SplicePath("a", "b"), "a/b");
#endif
}

TEST(TestUtil, testGetDouble)
{
    json_t json;
    rapidjson::Document d;
    d.Parse("{\n"
            "        \"ph\": \"X\",\n"
            "        \"name\": \"contiguous_d_Reshape\",\n"
            "        \"pid\": 768209,\n"
            "        \"tid\": 768209,\n"
            "        \"ts\": \"1699579270364817.47\",\n"
            "        \"dur\": \"169.33\",\n"
            "        \"cat\": \"cpu_op\",\n"
            "        \"args\": {}\n"
            "}");
    d.GetAllocator();
    double ts = JsonUtil::GetDouble(d, "ts");
    double dur = JsonUtil::GetDouble(d, "dur");
    EXPECT_EQ(ts, 1699579270364817.47);
    EXPECT_EQ(dur, 169.33);
}