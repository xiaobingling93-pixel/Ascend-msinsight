/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "GlobalDefs.h"
#include "SystemMemoryDatabaseDef.h"

using namespace Dic;
using namespace Dic::Module::Global;

class CommFuncTest : public testing::Test {
};

TEST_F(CommFuncTest, CastParserTypeToStr)
{
    EXPECT_EQ(CastParserTypeToStr(ParserType::JSON), "JSON");
    EXPECT_EQ(CastParserTypeToStr(ParserType::OTHER), "OTHER");
    EXPECT_EQ(CastParserTypeToStr(ParserType::BIN), "BIN");
    EXPECT_EQ(CastParserTypeToStr(ParserType::DB), "DB");
}

TEST_F(CommFuncTest, CastParseFileTypeToStr)
{
    EXPECT_EQ(CastParseFileTypeToStr(ParseFileType::RANK), "RANK");
    EXPECT_EQ(CastParseFileTypeToStr(ParseFileType::PROJECT), "PROJECT");
    EXPECT_EQ(CastParseFileTypeToStr(ParseFileType::DATA_FILE), "DATA_FILE");
    EXPECT_EQ(CastParseFileTypeToStr(ParseFileType::COMPUTE), "COMPUTE");
    EXPECT_EQ(CastParseFileTypeToStr(ParseFileType::CLUSTER), "CLUSTER");
}
