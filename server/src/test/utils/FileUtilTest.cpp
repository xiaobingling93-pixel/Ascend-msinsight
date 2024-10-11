/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "FileUtil.h"
#include "IdBuilder.h"
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

TEST(TestUtil, TestSplitToRankList)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(0, index + 1);
    std::vector<std::pair<std::string, std::string>> fileList;
    std::pair<std::string, std::string> pair1;
    pair1.first = "1";
    pair1.second = currPath + "/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/trace_view.json";
    fileList.push_back(pair1);
    std::pair<std::string, std::string> pair2;
    pair1.first = "0";
    pair1.second = currPath + "/src/test/test_data/test_rank_0/ASCEND_PROFILER_OUTPUT/trace_view.json";
    fileList.push_back(pair2);
    std::map<std::string, std::vector<std::string>> result = FileUtil::SplitToRankList(fileList);
    EXPECT_EQ(result.size(), 2);
}

TEST(TestUtil, TestGetRankIdFromFile)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(0, index + 1);
    std::string rank = FileUtil::GetRankIdFromFile(
            currPath + "/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/trace_view.json");
    EXPECT_EQ(rank, "1");
}

TEST(TestUtil, TestGetRankIdFromPath)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(0, index + 1);
    std::string rank = FileUtil::GetRankIdFromPath(
            currPath + "/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/trace_view.json");
    bool result = FileUtil::CheckFilePath(
            currPath + "/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/trace_view.json");
    EXPECT_EQ(rank, "test_rank_1");
    EXPECT_EQ(result, true);
}

TEST(TestUtil, TestGetDbPath)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(0, index + 1);
#ifdef _WIN32
    std::string dbPath = FileUtil::GetDbPath(
            currPath + "\\src\\test\\test_data\\test_rank_1\\ASCEND_PROFILER_OUTPUT\\trace_view.json", "1");
    EXPECT_EQ(dbPath,
            currPath + "\\src\\test\\test_data\\test_rank_1\\ASCEND_PROFILER_OUTPUT\\mindstudio_insight_data.db");
#else
    std::string dbPath = FileUtil::GetDbPath(
            currPath + "/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/trace_view.json", "1");
    EXPECT_EQ(dbPath, currPath + "/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT/mindstudio_insight_data.db");
#endif
}

TEST(TestUtil, TestIdBuilder)
{
    int id2 = IdBuilder::EventIdBuilder().Build();
    int id3 = IdBuilder::RequestIdBuilder().Build();
    int id4 = IdBuilder::SessionIdBuilder().Build();
    EXPECT_EQ(id2, 0);
    EXPECT_EQ(id3, 0);
    EXPECT_EQ(id4, 0);
}

TEST(TestUtil, TestGetFileSizeNullFileName)
{
    auto res = FileUtil::GetFileSize(nullptr);
    EXPECT_EQ(res, 0);
}

TEST(TestUtil, TestIsAbsolutePathEmtpyPath)
{
    EXPECT_EQ(FileUtil::IsAbsolutePath(""), false);
}