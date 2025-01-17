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
        SourceInstructionParser::ConvertApiInstrDynamic(json);
    }

    std::vector<SourceFileInstructionDynamicCol>& GetInstructionList()
    {
        return SourceInstructionParser::GetInstructionList();
    }

    void ConvertApiFileNew(std::string &json)
    {
        SourceInstructionParser::ConvertApiFileDynamic(json);
    }

    std::unordered_map<std::string, std::vector<SourceFileLineDynamicCol>>& GetSourceLinesMap()
    {
        return SourceInstructionParser::GetSourceLinesMap();
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

TEST(SourceInstructionParserTest, testConvertApiFileNewWithValidJsonData)
{
    SourceInstructionParserDerived parser;
    std::string json = std::string(SOURCE_API_FILE_JSON);
    parser.ConvertApiFileNew(json);
    auto map = parser.GetSourceLinesMap();
    EXPECT_EQ(map.size(), 2);
    EXPECT_TRUE(map.count("b.cpp") != 0);
    auto lines = map["b.cpp"];
    EXPECT_EQ(lines.size(), 2);
    auto line = lines[1];
    EXPECT_EQ(line.addressRange.size(), 2);
    EXPECT_EQ(line.addressRange[1].first, "ag");
    EXPECT_EQ(line.addressRange[1].second, "ah");

    EXPECT_EQ(line.stringColumnMap["string"].size(), 1);
    EXPECT_EQ(line.stringColumnMap["string"][0], "0x1269f001");
    EXPECT_EQ(line.intColumnMap["int"].size(), 1);
    EXPECT_EQ(line.intColumnMap["int"][0], 11);
    EXPECT_EQ(line.floatColumnMap["float"].size(), 1);
    EXPECT_TRUE(NumberUtil::IsEqual(line.floatColumnMap["float"][0], 1.2));
    EXPECT_EQ(line.stringColumnMap["string list"].size(), 2);
    EXPECT_EQ(line.stringColumnMap["string list"][1], "bb1");
    EXPECT_EQ(line.intColumnMap["int list"].size(), 2);
    EXPECT_EQ(line.intColumnMap["int list"][1], 22);
    EXPECT_EQ(line.floatColumnMap["float list"].size(), 2);
    EXPECT_TRUE(NumberUtil::IsEqual(line.floatColumnMap["float list"][1], 12.2));
}