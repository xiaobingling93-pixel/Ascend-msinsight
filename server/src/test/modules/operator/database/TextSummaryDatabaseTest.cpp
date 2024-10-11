/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "FileUtil.h"
#include "TableDefs.h"
#include "ConstantDefs.h"
#include "TextSummaryDataBase.h"

using namespace Dic;
using namespace Dic::Module::Summary;

std::string g_testDbPath;
std::recursive_mutex g_testMutex;
TextSummaryDataBase g_testDataBase = TextSummaryDataBase(g_testMutex);

class TextSummaryDatabaseTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        g_testDbPath = currPath + R"(/src/test/test_data/test_text_database.db)";
        g_testDataBase.SetDbPath(g_testDbPath);
        g_testDataBase.OpenDb(g_testDbPath, false);
    }

    static void TearDownTestSuite()
    {
        g_testDataBase.CloseDb();
        if (FileUtil::CheckFilePathExist(g_testDbPath)) {
            FileUtil::RemoveFile(g_testDbPath);
        }
    }
protected:
    void SetUp() override
    {
        g_testDataBase.CreateTable();
        g_testDataBase.InitStmt();
    }

    void TearDown() override
    {
        g_testDataBase.DropAllTable();
        g_testDataBase.ReleaseStmt();
    }
};


TEST_F(TextSummaryDatabaseTest, CreateAndDropTableSuccess)
{
    bool result = g_testDataBase.CheckTableExist(TABLE_KERNEL);
    EXPECT_EQ(result, true);
    result = g_testDataBase.DropTable();
    EXPECT_EQ(result, true);
    result = g_testDataBase.CheckTableExist(TABLE_KERNEL);
    EXPECT_EQ(result, false);
    result = g_testDataBase.CreateTable();
    EXPECT_EQ(result, true);
    result = g_testDataBase.CheckTableExist(TABLE_KERNEL);
    EXPECT_EQ(result, true);
}

TEST_F(TextSummaryDatabaseTest, InitAndReleaseStmtSuccess)
{
    bool result = g_testDataBase.InitStmt();
    EXPECT_EQ(result, true);
    result = g_testDataBase.InitStmt();
    EXPECT_EQ(result, true);
    g_testDataBase.ReleaseStmt();
    // Initial失败暂时未构造出来
}

TEST_F(TextSummaryDatabaseTest, UpdateParseStatusAndCheckSuccess)
{
    bool result = g_testDataBase.UpdateParseStatus(NOT_FINISH_STATUS);
    EXPECT_EQ(result, true);
    result = g_testDataBase.HasFinishedParseLastTime();
    EXPECT_EQ(result, false);
    result = g_testDataBase.UpdateParseStatus(FINISH_STATUS);
    EXPECT_EQ(result, true);
    result = g_testDataBase.HasFinishedParseLastTime();
    EXPECT_EQ(result, true);
}

TEST_F(TextSummaryDatabaseTest, SaveKernelDetailAndCheckSuccess)
{
    for (int i = 0; i < 1000 / 2; ++i) { // 1000 is cacheSize, 2 is half of cacheSize
        Kernel kernel = {
            "rank" + std::to_string(i), "step" + std::to_string(i), "name" + std::to_string(i),
            "type" + std::to_string(i), "Dynamic", "AICore", 50 + i, 100.0 + i, 10.0 + i, i,
            "", "", "", "", "", ""
        };
        g_testDataBase.InsertKernelDetail(kernel);
    }
    int64_t num = 1;
    bool result = g_testDataBase.QueryTotalNumByAcceleratorCore("AICore", num);
    EXPECT_EQ(result, true);
    EXPECT_EQ(num, 0);
    g_testDataBase.SaveKernelDetail();
    result = g_testDataBase.QueryTotalNumByAcceleratorCore("AICore", num);
    EXPECT_EQ(result, true);
    EXPECT_EQ(num, 1000 / 2); // 1000 is cacheSize, 2 is half of cacheSize
}

TEST_F(TextSummaryDatabaseTest, InsertKernelDetailAndCheckSuccess)
{
    for (int i = 0; i < 1000 / 2; ++i) { // 1000 is cacheSize, 2 is half of cacheSize
        Kernel kernel = {
            "rank" + std::to_string(i), "step" + std::to_string(i), "name" + std::to_string(i),
            "type" + std::to_string(i), "Dynamic", "AICore", 50 + i, 100.0 + i, 10.0 + i, i,
            "", "", "", "", "", ""
        };
        g_testDataBase.InsertKernelDetail(kernel);
    }
    int64_t num = 1;
    bool result = g_testDataBase.QueryTotalNumByAcceleratorCore("AICore", num);
    EXPECT_EQ(result, true);
    EXPECT_EQ(num, 0);
    for (int i = 0; i < 1000; ++i) { // 1000 is cacheSize, 2 is half of cacheSize
        Kernel kernel = {
            "rank" + std::to_string(i), "step" + std::to_string(i), "name" + std::to_string(i),
            "type" + std::to_string(i), "Dynamic", "AICore", 50 + i, 100.0 + i, 10.0 + i, i,
            "", "", "", "", "", ""
        };
        g_testDataBase.InsertKernelDetail(kernel);
    }
    result = g_testDataBase.QueryTotalNumByAcceleratorCore("AICore", num);
    EXPECT_EQ(result, true);
    EXPECT_EQ(num, 1000); // 1000 is cacheSize
}

