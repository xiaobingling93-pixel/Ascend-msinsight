/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "DatabaseTest.cpp"
#include "TextTraceDatabase.h"

class TextTraceDatabaseTest : DatabaseTest {
};

TEST_F(DatabaseTest, OpenDb)
{
    std::recursive_mutex sqlMutex;
    Dic::Module::Timeline::TextTraceDatabase database(sqlMutex);
    database.OpenDb("tttt", true);
    database.CloseDb();
}
TEST_F(DatabaseTest, InitStmt)
{
    std::recursive_mutex sqlMutex;
    Dic::Module::Timeline::TextTraceDatabase database(sqlMutex);
    bool success = database.InitStmt();
    EXPECT_EQ(success, false);
}

TEST_F(DatabaseTest, CreateTable)
{
    std::recursive_mutex sqlMutex;
    Dic::Module::Timeline::TextTraceDatabase database(sqlMutex);
    database.CreateTable();

    database.OpenDb("tttt", true);
    bool success = database.CreateTable();
    database.CloseDb();
    EXPECT_EQ(success, true);
}

TEST_F(DatabaseTest, DropTable)
{
    std::recursive_mutex sqlMutex;
    Dic::Module::Timeline::TextTraceDatabase database(sqlMutex);
    bool success = database.DropTable();
    EXPECT_EQ(success, false);
}

TEST_F(DatabaseTest, CreateIndex)
{
    std::recursive_mutex sqlMutex;
    Dic::Module::Timeline::TextTraceDatabase database(sqlMutex);
    database.CreateIndex();

    database.OpenDb("tttt", true);
    bool success = database.CreateIndex();
    database.CloseDb();
    EXPECT_EQ(success, true);
}

TEST_F(DatabaseTest, InsertSlice)
{
    std::recursive_mutex sqlMutex;
    const int size = 1000;
    Dic::Module::Timeline::TextTraceDatabase database(sqlMutex);
    bool success = true;
    for (int i = 0; i < size; i++) {
        Dic::Module::Timeline::Trace::Slice event;
        success = success && database.InsertSlice(event);
    }
    EXPECT_EQ(success, true);
}


TEST_F(DatabaseTest, InsertFlow)
{
    std::recursive_mutex sqlMutex;
    Dic::Module::Timeline::TextTraceDatabase database(sqlMutex);
    bool success = true;
    const int size = 1000;
    for (int i = 0; i < size; i++) {
        Dic::Module::Timeline::Trace::Flow event;
        success = success && database.InsertFlow(event);
    }
    EXPECT_EQ(success, true);
}

TEST_F(DatabaseTest, InsertCounter)
{
    std::recursive_mutex sqlMutex;
    Dic::Module::Timeline::TextTraceDatabase database(sqlMutex);
    bool success = true;
    const int size = 1000;
    for (int i = 0; i < size; i++) {
        Dic::Module::Timeline::Trace::Counter event;
        success = success && database.InsertCounter(event);
    }
    EXPECT_EQ(success, true);
}

