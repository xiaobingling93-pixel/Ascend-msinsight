/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#include "SourceInstructionParserTest.h"
#include <gtest/gtest.h>
#include "SourceInstructionParser.h"
#include "NumberUtil.h"
#include "../handler/ComputeSourceFile.h"

using namespace Dic;
using namespace Dic::Module::Source;
using namespace Dic::Module::Source::Test;

class SourceInstructionParserTest : public ::testing::Test {};

class SourceInstructionParserDerived : public SourceInstructionParser {
public:
    void ConvertApiInstr(std::string &json)
    {
        SourceInstructionParser::ConvertApiInstr(json);
    }

    void ConvertApiInstrDynamic(std::string &json)
    {
        SourceInstructionParser::ConvertApiInstrDynamic(json);
    }

    std::vector<SourceFileInstructionDynamicCol>& GetInstructionList()
    {
        return SourceInstructionParser::GetInstructionList();
    }

    void ConvertApiFile(std::string &json)
    {
        SourceInstructionParser::ConvertApiFile(json);
    }

    void ConvertApiFileDynamic(std::string &json)
    {
        SourceInstructionParser::ConvertApiFileDynamic(json);
    }

    std::unordered_map<std::string, std::vector<SourceFileLineDynamicCol>>& GetSourceLinesMap()
    {
        return SourceInstructionParser::GetSourceLinesMap();
    }

    std::vector<SourceApiInstruction>& GetApiInstructionList()
    {
        return SourceInstructionParser::GetApiInstructionList();
    }

    std::map<std::string, std::vector<SourceFileLine>>& GetApiFiles()
    {
        return SourceInstructionParser::GetApiFiles();
    }
};

TEST(SourceInstructionParserTest, testConvertApiInstrNewWithValidJsonData)
{
    SourceInstructionParserDerived parser;
    std::string json = std::string(SOURCE_INSTRUCTIONS_JSON);
    parser.ConvertApiInstrDynamic(json);
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
    parser.ConvertApiFileDynamic(json);
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

TEST(SourceInstructionParserTest, testConvertApiInstrNewWithoutDtype)
{
    SourceInstructionParserDerived parser;
    std::string json = std::string(INSTR_FILE);
    parser.ConvertApiInstr(json);
    auto list = parser.GetApiInstructionList();
    EXPECT_TRUE(!list.empty());
    auto line = list[0];
    EXPECT_EQ(line.address, "0x1134e2d8");
    EXPECT_EQ(line.source, "SIMT_LDG [PEX:6|P],[Rn:0|R],[Rn1:1|R],[Rd:0|R],[#ofst:9],[cop:1],[l2_cache_hint:0]");
    EXPECT_EQ(line.pipe, "RVECLD");
    EXPECT_EQ(line.ascendCInnerCode, "/test/vec_add1_simt.cpp:50");
    EXPECT_TRUE(!line.realStallCycles.empty());
    EXPECT_EQ(line.realStallCycles[0], 13); // value is 13
    EXPECT_TRUE(!line.theoreticalStallCycles.empty());
    EXPECT_EQ(line.theoreticalStallCycles[0], 8); // value is 8
    EXPECT_TRUE(!line.cycles.empty());
    EXPECT_EQ(line.cycles[0], 62); // value is 62
    EXPECT_TRUE(!line.instructionsExecuted.empty());
    EXPECT_EQ(line.instructionsExecuted[0], 4); // value is 4
}

TEST(SourceInstructionParserTest, testConvertApiFileNewWithoutDtype)
{
    SourceInstructionParserDerived parser;
    std::string json = std::string(API_FILE);
    std::string sourceName = "/test/vec_add1_simt.cpp";
    parser.ConvertApiFile(json);
    auto map = parser.GetApiFiles();
    auto lines = map[sourceName];
    EXPECT_TRUE(!lines.empty());
    auto line = lines[0];
    EXPECT_EQ(line.line, 31); // value is 31
    EXPECT_TRUE(!line.instructionsExecuted.empty());
    EXPECT_EQ(line.instructionsExecuted[0], 8); // value is 8
    EXPECT_TRUE(!line.cycles.empty());
    EXPECT_EQ(line.cycles[0], 56); // value is 56
    EXPECT_TRUE(!line.addressRange.empty());
    EXPECT_EQ(line.addressRange[0].first, "0x1134e2d8");
    EXPECT_EQ(line.addressRange[0].second, "0x1134e4d8");
}