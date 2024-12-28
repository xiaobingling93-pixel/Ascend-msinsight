/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "ClusterFileParser.h"
#include "DbClusterDataBase.h"
#include "CommunicationMatrixRapidHandler.h"
#include "CommunicationRapidSaxHandler.h"
#include "../../TestSuit.cpp"

class ClusterParseTest : public ::testing::Test {
};

TEST_F(ClusterParseTest, ParserDbClusterFailWithEmptyPath)
{
    std::recursive_mutex mutex;
    std::string clusterDbPath;
    auto database = std::make_shared<Dic::Module::FullDb::DbClusterDataBase>(mutex);
    ClusterFileParser clusterFileParser(clusterDbPath, database, "Test");
    bool res = clusterFileParser.ParserClusterOfDb();
    EXPECT_EQ(res, false);
}

TEST_F(ClusterParseTest, MatrixRapidHandlerTest)
{
    CommunicationMatrixRapidHandler handler(nullptr, "TEST");
    rapidjson::SizeType elementCount;
    EXPECT_EQ(handler.Null(), true);
    EXPECT_EQ(handler.Bool(true), true);
    EXPECT_EQ(handler.Int(0), true);
    EXPECT_EQ(handler.Uint(0), true);
    EXPECT_EQ(handler.Int64(0), true);
    EXPECT_EQ(handler.Uint64(0), true);
    EXPECT_EQ(handler.StartArray(), true);
    EXPECT_EQ(handler.EndArray(elementCount), true);
}

TEST_F(ClusterParseTest, RapidSaxHandlerTest)
{
    CommunicationRapidSaxHandler handler(nullptr, "TEST");
    EXPECT_EQ(handler.Null(), true);
    EXPECT_EQ(handler.Bool(true), true);
    EXPECT_EQ(handler.Int(0), true);
    EXPECT_EQ(handler.Uint(0), true);
    EXPECT_EQ(handler.Int64(0), true);
    EXPECT_EQ(handler.Uint64(0), true);
    EXPECT_EQ(handler.Double(0), true);
}