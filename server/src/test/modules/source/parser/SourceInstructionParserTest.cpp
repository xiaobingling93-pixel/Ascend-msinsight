/*
* Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

#include "SourceInstructionParserTest.h"
#include <gtest/gtest.h>
#include "SourceInstructionParser.h"
#include "NumberUtil.h"

using namespace Dic;
using namespace Dic::Module::Source;

class SourceInstructionParserTest : public ::testing::Test {};

class SourceInstructionParserDerived : public SourceInstructionParser {
public:
    void ConvertApiInstrNew(std::string &json)
    {
        SourceInstructionParser::ConvertApiInstrNew(json);
    }

    std::vector<SourceFileInstruction>& GetInstructionList()
    {
        return SourceInstructionParser::GetInstructionList();
    }
};

TEST(SourceInstructionParserTest, testConvertApiInstrNewWithValidJsonData)
{
    SourceInstructionParserDerived parser;
    std::string json = std::string(SOURCE_INSTRUCTIONS_JSON);
    parser.ConvertApiInstrNew(json);
    auto list = parser.GetInstructionList();
    EXPECT_TRUE(list.size() == 2);
    auto instr = list[1];
    EXPECT_EQ(instr.stringColumnMap["string"].size(), 1);
    EXPECT_EQ(instr.stringColumnMap["string"][0], "0x1269f001");
    EXPECT_EQ(instr.stringColumnMap["string list"].size(), 2);
    EXPECT_EQ(instr.stringColumnMap["string list"][1], "bb1");

    EXPECT_EQ(instr.intColumnMap["int"].size(), 1);
    EXPECT_EQ(instr.intColumnMap["int"][0], 11);
    EXPECT_EQ(instr.intColumnMap["int list"].size(), 2);
    EXPECT_EQ(instr.intColumnMap["int list"][1], 22);

    EXPECT_EQ(instr.floatColumnMap["float"].size(), 1);

    EXPECT_TRUE(NumberUtil::IsEqual(instr.floatColumnMap["float"][0], 1.2));
    EXPECT_EQ(instr.floatColumnMap["float list"].size(), 2);
    EXPECT_TRUE(NumberUtil::IsEqual(instr.floatColumnMap["float list"][1], 12.2));
}