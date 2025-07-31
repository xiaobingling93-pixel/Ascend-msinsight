/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "Database.h"
#include "../../../DatabaseTestCaseMockUtil.cpp"
using namespace Dic::Global::PROFILER::MockUtil;
class DataBaseTest : public ::testing::Test {
protected:
    class MockDatabase : public Dic::Module::Database {
    public:
        explicit MockDatabase(std::recursive_mutex& sqlMutex) : Database(sqlMutex)
        {
        }
        void SetDbPtr(sqlite3* dbPtr)
        {
            isOpen = true;
            db = dbPtr;
            path = ":memory:";
        }
    };
};

/**
 * 根据表名查询表结构，正常情况
 */
TEST_F(DataBaseTest, TestQueryTableInfoByNameNormal)
{
    std::recursive_mutex sqlMutex;
    MockDatabase database(sqlMutex);
    sqlite3* dbPtr = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(dbPtr);
    database.SetDbPtr(dbPtr);
    std::string sql = "CREATE TABLE \"decode_gen_speed\" (\n"
                      "\"timestamp\" TEXT,\n"
                      "  \"avg\" REAL,\n"
                      "  \"p99\" REAL,\n"
                      "  \"p90\" REAL,\n"
                      "  \"p50\" REAL\n"
                      ");";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, sql);
    auto res = database.QueryTableInfoByName("decode_gen_speed");
    const uint64_t expectSize = 5;
    EXPECT_EQ(res.size(), expectSize);
    EXPECT_EQ(res.front().key, "timestamp");
    EXPECT_EQ(res.front().type, "TEXT");
    EXPECT_EQ(res.back().key, "p50");
    EXPECT_EQ(res.back().type, "REAL");
}

/**
 * 根据表名查询表结构，sql注入情况
 */
TEST_F(DataBaseTest, TestQueryTableInfoByNameWhenSqlInjectReturnEmpty)
{
    std::recursive_mutex sqlMutex;
    MockDatabase database(sqlMutex);
    sqlite3* dbPtr = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(dbPtr);
    database.SetDbPtr(dbPtr);
    std::string sql = "CREATE TABLE \"decode_gen_speed\" (\n"
                      "\"timestamp\" TEXT,\n"
                      "  \"avg\" REAL,\n"
                      "  \"p99\" REAL,\n"
                      "  \"p90\" REAL,\n"
                      "  \"p50\" REAL\n"
                      ");";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, sql);
    auto res = database.QueryTableInfoByName("decode_gen_speed()");
    const uint64_t expectSize = 0;
    EXPECT_EQ(res.size(), expectSize);
}

/**
 * 根据表名查询表结构，表名不正确
 */
TEST_F(DataBaseTest, TestQueryTableInfoByNameWhenNameErrReturnEmpty)
{
    std::recursive_mutex sqlMutex;
    MockDatabase database(sqlMutex);
    sqlite3* dbPtr = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(dbPtr);
    database.SetDbPtr(dbPtr);
    std::string sql = "CREATE TABLE \"decode_gen_speed\" (\n"
                      "\"timestamp\" TEXT,\n"
                      "  \"avg\" REAL,\n"
                      "  \"p99\" REAL,\n"
                      "  \"p90\" REAL,\n"
                      "  \"p50\" REAL\n"
                      ");";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, sql);
    auto res = database.QueryTableInfoByName("SELECT");
    const uint64_t expectSize = 0;
    EXPECT_EQ(res.size(), expectSize);
}

/**
 * 根据表名查询表结构，db未打开
 */
TEST_F(DataBaseTest, TestQueryTableInfoByNameWhenDbNotOpenReturnEmpty)
{
    std::recursive_mutex sqlMutex;
    MockDatabase database(sqlMutex);
    sqlite3* dbPtr = nullptr;
    database.SetDbPtr(dbPtr);
    auto res = database.QueryTableInfoByName("SELECT");
    const uint64_t expectSize = 0;
    EXPECT_EQ(res.size(), expectSize);
}

/**
 * 根据表名查询表数据量，正常情况
 */
TEST_F(DataBaseTest, TestQueryCountByTableNameNormal)
{
    std::recursive_mutex sqlMutex;
    MockDatabase database(sqlMutex);
    sqlite3* dbPtr = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(dbPtr);
    database.SetDbPtr(dbPtr);
    std::string sql = "CREATE TABLE \"decode_gen_speed\" (\n"
                      "\"timestamp\" TEXT,\n"
                      "  \"avg\" REAL,\n"
                      "  \"p99\" REAL,\n"
                      "  \"p90\" REAL,\n"
                      "  \"p50\" REAL\n"
                      ");";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, sql);
    std::string dataSql = "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:735760', 6.0869, 6.1759, 6.1512, 6.041);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:44:344296', 2.1341, 2.1451, 2.142, 2.1285);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:736910', 6.0657, 6.1541, 6.1295, 6.0201);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:736935', 6.0652, 6.1537, 6.129, 6.0196);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:736967', 6.0646, 6.1531, 6.1285, 6.0191);";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, dataSql);
    Dic::Module::PageQuery query;
    const std::vector<Dic::Module::ColumnAtt> columns = database.QueryTableInfoByName("decode_gen_speed");
    query.viewName = "decode_gen_speed";
    auto res = database.QueryCountByTableName(query, columns);
    const uint64_t expectSize = 5;
    EXPECT_EQ(res, expectSize);
}

/**
 * 根据表名查询表数据量，搜索情况
 */
TEST_F(DataBaseTest, TestQueryCountByTableNameSearch)
{
    std::recursive_mutex sqlMutex;
    MockDatabase database(sqlMutex);
    sqlite3* dbPtr = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(dbPtr);
    database.SetDbPtr(dbPtr);
    std::string sql = "CREATE TABLE \"decode_gen_speed\" (\n"
                      "\"timestamp\" TEXT,\n"
                      "  \"avg\" REAL,\n"
                      "  \"p99\" REAL,\n"
                      "  \"p90\" REAL,\n"
                      "  \"p50\" REAL\n"
                      ");";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, sql);
    std::string dataSql = "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:735760', 6.0869, 6.1759, 6.1512, 6.041);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:44:344296', 2.1341, 2.1451, 2.142, 2.1285);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:736910', 6.0657, 6.1541, 6.1295, 6.0201);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:736935', 6.0652, 6.1537, 6.129, 6.0196);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:736967', 6.0646, 6.1531, 6.1285, 6.0191);";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, dataSql);
    Dic::Module::PageQuery query;
    const std::vector<Dic::Module::ColumnAtt> columns = database.QueryTableInfoByName("decode_gen_speed");
    query.viewName = "decode_gen_speed";
    query.pageFilters.push_back({"avg", "6.065"});
    auto res = database.QueryCountByTableName(query, columns);
    const uint64_t expectSize = 2;
    EXPECT_EQ(res, expectSize);
}

/**
 * 根据表名查询表数据量，sql注入情况
 */
TEST_F(DataBaseTest, TestQueryCountByTableNameWhenSqlInjectReturnEmpty)
{
    std::recursive_mutex sqlMutex;
    MockDatabase database(sqlMutex);
    sqlite3* dbPtr = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(dbPtr);
    database.SetDbPtr(dbPtr);
    std::string sql = "CREATE TABLE \"decode_gen_speed\" (\n"
                      "\"timestamp\" TEXT,\n"
                      "  \"avg\" REAL,\n"
                      "  \"p99\" REAL,\n"
                      "  \"p90\" REAL,\n"
                      "  \"p50\" REAL\n"
                      ");";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, sql);
    Dic::Module::PageQuery query;
    const std::vector<Dic::Module::ColumnAtt> columns = database.QueryTableInfoByName("decode_gen_speed()");
    query.viewName = "decode_gen_speed()";
    auto res = database.QueryCountByTableName(query, columns);
    const uint64_t expectSize = 0;
    EXPECT_EQ(res, expectSize);
}

/**
 * 根据表名查询表数据量，表名不正确
 */
TEST_F(DataBaseTest, TestQueryCountByTableNameWhenNameErrReturnEmpty)
{
    std::recursive_mutex sqlMutex;
    MockDatabase database(sqlMutex);
    sqlite3* dbPtr = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(dbPtr);
    database.SetDbPtr(dbPtr);
    std::string sql = "CREATE TABLE \"decode_gen_speed\" (\n"
                      "\"timestamp\" TEXT,\n"
                      "  \"avg\" REAL,\n"
                      "  \"p99\" REAL,\n"
                      "  \"p90\" REAL,\n"
                      "  \"p50\" REAL\n"
                      ");";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, sql);
    Dic::Module::PageQuery query;
    const std::vector<Dic::Module::ColumnAtt> columns = database.QueryTableInfoByName("SELECT");
    query.viewName = "SELECT";
    auto res = database.QueryCountByTableName(query, columns);
    const uint64_t expectSize = 0;
    EXPECT_EQ(res, expectSize);
}

/**
 * 根据表名查询表数据量，db未打开
 */
TEST_F(DataBaseTest, TestQueryCountByTableNameWhenDbNotOpenReturnEmpty)
{
    std::recursive_mutex sqlMutex;
    MockDatabase database(sqlMutex);
    sqlite3* dbPtr = nullptr;
    database.SetDbPtr(dbPtr);
    Dic::Module::PageQuery query;
    const std::vector<Dic::Module::ColumnAtt> columns = database.QueryTableInfoByName("SELECT");
    query.viewName = "SELECT";
    auto res = database.QueryCountByTableName(query, columns);
    const uint64_t expectSize = 0;
    EXPECT_EQ(res, expectSize);
}

/**
 * 分页查询表数据，正常情况
 */
TEST_F(DataBaseTest, TestQueryDataByPageNormal)
{
    std::recursive_mutex sqlMutex;
    MockDatabase database(sqlMutex);
    sqlite3* dbPtr = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(dbPtr);
    database.SetDbPtr(dbPtr);
    std::string sql = "CREATE TABLE \"decode_gen_speed\" (\n"
                      "\"timestamp\" TEXT,\n"
                      "  \"avg\" REAL,\n"
                      "  \"p99\" REAL,\n"
                      "  \"p90\" REAL,\n"
                      "  \"p50\" REAL\n"
                      ");";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, sql);
    std::string dataSql = "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:646147', NULL, NULL, NULL, NULL);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:646138', NULL, NULL, NULL, NULL);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:735760', 6.0869, 6.1759, 6.1512, 6.041);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:44:344296', 2.1341, 2.1451, 2.142, 2.1285);";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, dataSql);
    auto cols = database.QueryTableInfoByName("decode_gen_speed");
    Dic::Module::PageQuery pageQuery;
    pageQuery.curPage = 1;  // 1
    pageQuery.size = 10;  // 10
    pageQuery.viewName = "decode_gen_speed";
    auto res = database.QueryDataByPage(pageQuery, cols);
    const uint64_t expectSize = 4;
    EXPECT_EQ(res.size(), expectSize);
    EXPECT_EQ(res.front()[cols.front().key], "2025-03-28 22:57:43:646147");
    EXPECT_EQ(res.front()[cols.back().key], "0");
    EXPECT_EQ(res.back()[cols.front().key], "2025-03-28 22:57:44:344296");
    EXPECT_EQ(res.back()[cols.back().key], "2.1285");
}

/**
 * 分页查询表数据，搜索情况
 */
TEST_F(DataBaseTest, TestQueryDataByPageSearch)
{
    std::recursive_mutex sqlMutex;
    MockDatabase database(sqlMutex);
    sqlite3* dbPtr = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(dbPtr);
    database.SetDbPtr(dbPtr);
    std::string sql = "CREATE TABLE \"decode_gen_speed\" (\n"
                      "\"timestamp\" TEXT,\n"
                      "  \"avg\" REAL,\n"
                      "  \"p99\" REAL,\n"
                      "  \"p90\" REAL,\n"
                      "  \"p50\" REAL\n"
                      ");";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, sql);
    std::string dataSql = "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:646147', NULL, NULL, NULL, NULL);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:646138', NULL, NULL, NULL, NULL);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:735760', 6.0869, 6.1759, 6.1512, 6.041);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:44:344296', 2.1341, 2.1451, 2.142, 2.1285);";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, dataSql);
    auto cols = database.QueryTableInfoByName("decode_gen_speed");
    Dic::Module::PageQuery pageQuery;
    pageQuery.curPage = 1;  // 1
    pageQuery.size = 10;  // 10
    pageQuery.viewName = "decode_gen_speed";
    pageQuery.pageFilters.push_back({"timestamp", "646"});
    auto res = database.QueryDataByPage(pageQuery, cols);
    const uint64_t expectSize = 2;
    EXPECT_EQ(res.size(), expectSize);
    EXPECT_EQ(res.front()[cols.front().key], "2025-03-28 22:57:43:646147");
    EXPECT_EQ(res.front()[cols.back().key], "0");
    EXPECT_EQ(res.back()[cols.front().key], "2025-03-28 22:57:43:646138");
    EXPECT_EQ(res.back()[cols.back().key], "0");
}

/**
 * 分页查询表数据，有排序条件
 */
TEST_F(DataBaseTest, TestQueryDataByPageWhenOrderAndLimit)
{
    std::recursive_mutex sqlMutex;
    MockDatabase database(sqlMutex);
    sqlite3* dbPtr = nullptr;
    DatabaseTestCaseMockUtil::OpenDB(dbPtr);
    database.SetDbPtr(dbPtr);
    std::string sql = "CREATE TABLE \"decode_gen_speed\" (\n"
                      "\"timestamp\" TEXT,\n"
                      "  \"avg\" REAL,\n"
                      "  \"p99\" REAL,\n"
                      "  \"p90\" REAL,\n"
                      "  \"p50\" REAL\n"
                      ");";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, sql);
    std::string dataSql = "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:735760', 6.0869, 6.1759, 6.1512, 6.041);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:44:344296', 2.1341, 2.1451, 2.142, 2.1285);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:736910', 6.0657, 6.1541, 6.1295, 6.0201);\n"
                          "INSERT INTO \"main\".\"decode_gen_speed\" (\"timestamp\", \"avg\", \"p99\", \"p90\", "
                          "\"p50\") VALUES ('2025-03-28 22:57:43:736935', 6.0652, 6.1537, 6.129, 6.0196);";
    DatabaseTestCaseMockUtil::InsertData(dbPtr, dataSql);
    auto cols = database.QueryTableInfoByName("decode_gen_speed");
    Dic::Module::PageQuery pageQuery;
    pageQuery.curPage = 1; // 1
    pageQuery.size = 2; // 2
    pageQuery.viewName = "decode_gen_speed";
    pageQuery.orderBy = "timestamp";
    pageQuery.order = "descend";
    auto res = database.QueryDataByPage(pageQuery, cols);
    const uint64_t expectSize = 2;
    EXPECT_EQ(res.size(), expectSize);
    EXPECT_EQ(res.front()[cols.front().key], "2025-03-28 22:57:44:344296");
    EXPECT_EQ(res.front()[cols.back().key], "2.1285");
    EXPECT_EQ(res.back()[cols.front().key], "2025-03-28 22:57:43:736935");
    EXPECT_EQ(res.back()[cols.back().key], "6.0196");
}