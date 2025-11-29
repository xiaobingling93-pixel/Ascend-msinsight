/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "../../DatabaseTestCaseMockUtil.h"
#include "DbClusterDataBase.h"

using namespace Dic::Global::PROFILER::MockUtil;
class DbClusterDataBaseTest : public ::testing::Test {
protected:
    class MockDatabase : public Dic::Module::FullDb::DbClusterDataBase {
    public:
        explicit MockDatabase(std::recursive_mutex &sqlMutex) : Dic::Module::FullDb::DbClusterDataBase(sqlMutex) {}
        void SetDbPtr(sqlite3 *dbPtr)
        {
            isOpen = true;
            db = dbPtr;
            path = ":memory:";
        }
    };
};

/**
 * DB场景设置并行策略,只设置了dp，pp，tp,兼容老版本
 */
TEST_F(DbClusterDataBaseTest, TestQueryParallelStrategyConfigWhenNotSetCpAndEp)
{
    std::recursive_mutex sqlMutex;
    sqlite3 *dbPtr = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(dbPtr);
    MockDatabase database(sqlMutex);
    database.SetDbPtr(dbPtr);
    database.CreateTable();
    const int64_t expectPP = 12;
    const int64_t expectTP = 11;
    const int64_t expectDP = 10;
    const int64_t expectCP = 1;
    const int64_t expectEP = 1;
    Dic::Module::ParallelStrategyConfig config = { "ttttt", expectPP, expectTP, expectDP };
    std::string msg;
    std::string level = "configed";
    Dic::Module::ClusterBaseInfo baseInfo;
    database.InsertClusterBaseInfo(baseInfo);
    database.UpdateParallelStrategyConfig(config, level, msg);
    Dic::Module::ParallelStrategyConfig result;
    std::string resultLevel;
    database.QueryParallelStrategyConfig(result, resultLevel);
    EXPECT_EQ(result.ppSize, expectPP);
    EXPECT_EQ(result.tpSize, expectTP);
    EXPECT_EQ(result.dpSize, expectDP);
    EXPECT_EQ(result.cpSize, expectCP);
    EXPECT_EQ(result.epSize, expectEP);
    EXPECT_EQ(resultLevel, level);
}

/**
 * DB场景设置并行策略,设置了dp，pp，tp,cp,ep
 */
TEST_F(DbClusterDataBaseTest, TestQueryParallelStrategyConfigWhenSetAllConfig)
{
    std::recursive_mutex sqlMutex;
    sqlite3 *dbPtr = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(dbPtr);
    MockDatabase database(sqlMutex);
    database.SetDbPtr(dbPtr);
    database.CreateTable();
    const int64_t expectPP = 12;
    const int64_t expectTP = 11;
    const int64_t expectDP = 10;
    const int64_t expectCP = 18;
    const int64_t expectEP = 17;
    Dic::Module::ParallelStrategyConfig config = { "ttttt", expectPP, expectTP, expectDP, expectCP, expectEP };
    std::string msg;
    std::string level = "configed";
    Dic::Module::ClusterBaseInfo baseInfo;
    database.InsertClusterBaseInfo(baseInfo);
    database.UpdateParallelStrategyConfig(config, level, msg);
    Dic::Module::ParallelStrategyConfig result;
    std::string resultLevel;
    database.QueryParallelStrategyConfig(result, resultLevel);
    EXPECT_EQ(result.ppSize, expectPP);
    EXPECT_EQ(result.tpSize, expectTP);
    EXPECT_EQ(result.dpSize, expectDP);
    EXPECT_EQ(result.cpSize, expectCP);
    EXPECT_EQ(result.epSize, expectEP);
    EXPECT_EQ(resultLevel, level);
}

/**
 * DB场景查询并行策略,db未打开
 */
TEST_F(DbClusterDataBaseTest, TestQueryParallelStrategyConfigWhenDbNotOpen)
{
    std::recursive_mutex sqlMutex;
    MockDatabase database(sqlMutex);
    const int64_t expectPP = 0;
    const int64_t expectTP = 0;
    const int64_t expectDP = 0;
    const int64_t expectCP = 1;
    const int64_t expectEP = 1;
    Dic::Module::ParallelStrategyConfig result;
    std::string resultLevel;
    bool ans = database.QueryParallelStrategyConfig(result, resultLevel);
    EXPECT_EQ(ans, false);
    EXPECT_EQ(result.ppSize, expectPP);
    EXPECT_EQ(result.tpSize, expectTP);
    EXPECT_EQ(result.dpSize, expectDP);
    EXPECT_EQ(result.cpSize, expectCP);
    EXPECT_EQ(result.epSize, expectEP);
    EXPECT_EQ(resultLevel, "");
}

/**
 * DB场景设置并行策略,db未打开
 */
TEST_F(DbClusterDataBaseTest, TestUpdateParallelStrategyConfigWhenDbNotOpen)
{
    std::recursive_mutex sqlMutex;
    MockDatabase database(sqlMutex);
    const int64_t expectPP = 12;
    const int64_t expectTP = 11;
    const int64_t expectDP = 10;
    const int64_t expectCP = 18;
    const int64_t expectEP = 17;
    Dic::Module::ParallelStrategyConfig config = { "ttttt", expectPP, expectTP, expectDP, expectCP, expectEP };
    std::string msg;
    std::string level = "configed";
    bool ans = database.UpdateParallelStrategyConfig(config, level, msg);
    EXPECT_EQ(ans, false);
}