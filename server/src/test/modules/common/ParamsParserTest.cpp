/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "ParamsParser.h"

class ParamsParserTest : public ::testing::Test {
};

TEST_F(ParamsParserTest, test1)
{
    Dic::Server::ParamsParser::Instance();
    Dic::Server::ParamsParser::Instance().Parse({"exe", "--wsPort=9000", "--wsHost=127.0.0.1", "--scan=3000",
                                                 "--logPath=./", "--logSize=10", "--logLevel=INFO", "--sid=sid"});
    int64_t expectPort = 9000;
    int64_t expectScanPort = 3000;
    int64_t expectLogSize = 10;
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetOption().wsPort, expectPort);
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetOption().scanPort, expectScanPort);
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetOption().logSize, expectLogSize);
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetOption().host, "127.0.0.1");
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetOption().sid, "sid");
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetOption().logLevel, "INFO");
    EXPECT_EQ(Dic::Server::ParamsParser::Instance().GetOption().logPath, "./");
}
