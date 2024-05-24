/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "SourceFileParser.h"
#include "QueryApiInstructionsHandler.h"
#include "QueryApiLineHandler.h"
#include "QueryCodeFileHandler.h"
#include "SourceProtocolRequest.h"
#include "WsSessionManager.h"
#include "StringUtil.h"
#include "ParserFactory.h"
#include "../../TestSuit.cpp"

class SourceTest : TestSuit {};

static const int NUM0 = 0;
static const int NUM1 = 1;
static const int NUM2 = 2;
static const int NUM3 = 3;
static const int NUM5 = 5;
static const int NUM6 = 6;
static const int NUM7 = 7;
static const int NUM32 = 32;
static const int NUM193 = 193;
static const int NUM729 = 729;
static const int NUM16293 = 16293;

TEST_F(TestSuit, SourceFileParser)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(NUM0, index + 1);
    std::string selectedFolder = currPath + R"(/src/test/test_data/data.bin)";

    Module::Source::SourceFileParser &parser = Dic::Module::Source::SourceFileParser::Instance();
    EXPECT_EQ(true, parser.CheckOperatorBinary(selectedFolder));
    DataBaseManager::Instance().CreatConnectionPool("testFileId", "testFileId.db");
    parser.Parse(std::vector<std::string>(), "testFileId", selectedFolder);
    parser.ConvertToData();

    const vector<std::string> &coreList = parser.GetCoreList();
    EXPECT_EQ(coreList.size(), NUM3);
    EXPECT_EQ(coreList[NUM0], "core0.veccore0");
    EXPECT_EQ(coreList[NUM1], "core0.veccore1");
    EXPECT_EQ(coreList[NUM2], "core1.veccore0");
    const vector<std::string> &sourceList = parser.GetSourceList();
    EXPECT_EQ(sourceList.size(), NUM6);
    EXPECT_EQ(sourceList[NUM0], "/home/lantianxiang/workspace/foreach/common/foreach/foreach_tiling_def.h");
    EXPECT_EQ(sourceList[NUM3], "/home/lantianxiang/workspace/foreach/common/foreach/op_kernel/kernel_foreach_base.h");
    EXPECT_EQ(sourceList[NUM5], "/home/lantianxiang/workspace/foreach/foreach_sub_scalar_list.cpp");
    const vector<Dic::Module::Source::SourceFileLine> &linesByCoreAndSource = parser.GetApiLinesByCoreAndSource(
        "core0.veccore0", "/home/lantianxiang/workspace/foreach/common/foreach/foreach_tiling_def.h");
    EXPECT_EQ(linesByCoreAndSource.size(), NUM7);
    const string &instr = parser.GetInstr();
    EXPECT_TRUE(!instr.empty());
    const string &source =
        parser.GetSourceByName("/home/lantianxiang/workspace/foreach/common/foreach/foreach_tiling_def.h");
    EXPECT_TRUE(!source.empty());
    parser.Reset();
}

TEST_F(TestSuit, QueryApiInstructions)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(NUM0, index + 1);
    std::string selectedFolder = currPath + R"(/src/test/test_data/data.bin)";

    Module::Source::SourceFileParser &parser = Dic::Module::Source::SourceFileParser::Instance();
    EXPECT_EQ(true, parser.CheckOperatorBinary(selectedFolder));
    parser.Parse(std::vector<std::string>(), "", selectedFolder);
    parser.ConvertToData();

    const string &instr = parser.GetInstr();
    EXPECT_EQ(true, StringUtil::Contains(instr, "Cores"));
    EXPECT_EQ(true, StringUtil::Contains(instr, "core0.veccore0"));
    EXPECT_EQ(true, StringUtil::Contains(instr, "Address"));
    EXPECT_EQ(true, StringUtil::Contains(instr, "Instructions Executed"));
}

TEST_F(TestSuit, QueryApiLine)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(NUM0, index + 1);
    std::string selectedFolder = currPath + R"(/src/test/test_data/data.bin)";

    Module::Source::SourceFileParser &parser = Dic::Module::Source::SourceFileParser::Instance();
    EXPECT_EQ(true, parser.CheckOperatorBinary(selectedFolder));
    parser.Parse(std::vector<std::string>(), "", selectedFolder);
    parser.ConvertToData();

    vector<Dic::Module::Source::SourceFileLine> lines = parser.GetApiLinesByCoreAndSource("core0.veccore0",
        "/home/lantianxiang/workspace/foreach/common/foreach/foreach_tiling_def.h");
    EXPECT_EQ(NUM7, lines.size());
    EXPECT_EQ(NUM0, lines[NUM0].line);
    EXPECT_EQ(NUM729, lines[NUM0].cycles[NUM0]);
    EXPECT_EQ(NUM5, lines[NUM0].instructionsExecuted[NUM0]);
    EXPECT_EQ("0x1124406c", lines[NUM0].addressRange[NUM0].first);
    EXPECT_EQ("0x1124407c", lines[NUM0].addressRange[NUM0].second);

    EXPECT_EQ(NUM32, lines[NUM6].line);
    EXPECT_EQ(NUM16293, lines[NUM6].cycles[NUM0]);
    EXPECT_EQ(NUM193, lines[NUM6].instructionsExecuted[NUM0]);
    EXPECT_EQ("0x11244540", lines[NUM6].addressRange[NUM0].first);
    EXPECT_EQ("0x11244540", lines[NUM6].addressRange[NUM0].second);
    EXPECT_EQ("0x11244548", lines[NUM6].addressRange[1].first);
    EXPECT_EQ("0x11244844", lines[NUM6].addressRange[1].second);
}

TEST_F(TestSuit, QueryCodeFile)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(NUM0, index + 1);
    std::string selectedFolder = currPath + R"(/src/test/test_data/data.bin)";

    Module::Source::SourceFileParser &parser = Dic::Module::Source::SourceFileParser::Instance();
    EXPECT_EQ(true, parser.CheckOperatorBinary(selectedFolder));
    parser.Parse(std::vector<std::string>(), "", selectedFolder);
    parser.ConvertToData();

    string sourcefile =
        parser.GetSourceByName("/home/lantianxiang/workspace/foreach/common/foreach/foreach_tiling_def.h");
    EXPECT_EQ(true, StringUtil::Contains(sourcefile, "__FOREACH_TILING_DEF_H__"));
    EXPECT_EQ(true, StringUtil::Contains(sourcefile, "ForeachCommonTilingData"));
    EXPECT_EQ(true, StringUtil::Contains(sourcefile, "InitForeachCommonTilingData"));
    EXPECT_EQ(true, StringUtil::Contains(sourcefile, "endif"));
}

TEST_F(TestSuit, QueryBaseInfo)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(NUM0, index + 1);
    std::string selectedFolder = currPath + R"(/src/test/test_data/base_info.bin)";

    Module::Source::SourceFileParser &parser = Dic::Module::Source::SourceFileParser::Instance();
    EXPECT_EQ(true, parser.CheckOperatorBinary(selectedFolder));
    parser.Parse(std::vector<std::string>(), "", selectedFolder);

    Protocol::DetailsBaseInfoResBody resBody;
    bool res = parser.GetDetailsBaseInfo(resBody);
    EXPECT_EQ(true, res);
    EXPECT_EQ("sin_custom", resBody.name);
    EXPECT_EQ("Ascend910B4", resBody.soc);
    EXPECT_EQ("32", resBody.blockDim);
    EXPECT_EQ("13.0600004196167", resBody.duration);
    EXPECT_EQ(NUM32, resBody.blockDetail.size());
}

TEST_F(TestSuit, QueryLoadData)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(NUM0, index + 1);
    std::string selectedFolder = currPath + R"(/src/test/test_data/load_data.bin)";

    Module::Source::SourceFileParser &parser = Dic::Module::Source::SourceFileParser::Instance();
    EXPECT_EQ(true, parser.CheckOperatorBinary(selectedFolder));
    parser.Parse(std::vector<std::string>(), "", selectedFolder);

    Protocol::DetailsLoadInfoResBody resBody;
    bool res = parser.GetDetailsLoadInfo(resBody);
    EXPECT_EQ(true, res);
    EXPECT_EQ(NUM32, resBody.blockIdList.size());
}

TEST_F(TestSuit, QueryMemoryGraphDataMix)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(NUM0, index + 1);
    std::string selectedFolder = currPath + R"(/src/test/test_data/memory_data.bin)";

    Module::Source::SourceFileParser &parser = Dic::Module::Source::SourceFileParser::Instance();
    EXPECT_EQ(true, parser.CheckOperatorBinary(selectedFolder));
    parser.Parse(std::vector<std::string>(), "", selectedFolder);

    Protocol::DetailsMemoryGraphResBody resBody;
    bool res = parser.GetDetailsMemoryGraph("0", resBody);
    EXPECT_EQ(true, res);
}

TEST_F(TestSuit, QueryMemoryTableDataMix)
{
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    currPath = currPath.substr(NUM0, index + 1);
    std::string selectedFolder = currPath + R"(/src/test/test_data/memory_data.bin)";

    Module::Source::SourceFileParser &parser = Dic::Module::Source::SourceFileParser::Instance();
    EXPECT_EQ(true, parser.CheckOperatorBinary(selectedFolder));
    parser.Parse(std::vector<std::string>(), "", selectedFolder);

    Protocol::DetailsMemoryTableResBody resBody;
    bool res = parser.GetDetailsMemoryTable("0", resBody);
    EXPECT_EQ(true, res);
}