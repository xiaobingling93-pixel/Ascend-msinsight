/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "ProjectParserBin.h"
#include "../../source/mockUtils/BinFileGenerator.h"
#include "../../source/DetailsComputeLoadJson.h"

using namespace Dic::Module;
using namespace Dic::Module::Global;
using namespace Dic::Module::Source::Test;

class ParserBinTest : public ::testing::Test {
};

TEST_F(ParserBinTest, ParserWithValidData)
{
    std::string fileName = "ParserBinTest.bin";
    std::string filePath = "./" + fileName;

    // generate bin file
    NormalDataBlock baseInfoDataBlock(DataTypeEnum::DETAILS_BASE_INFO, DETAILS_BASE_INFO_JSON);
    BinFileGenerator generator;
    generator.AddDataBlock(std::make_unique<NormalDataBlock>(baseInfoDataBlock));
    generator.Generate(fileName);

    std::vector<std::shared_ptr<ParseFileInfo>> filePathInfos;
    auto fileInfo = std::make_shared<ParseFileInfo>();
    fileInfo->id = 1;
    fileInfo->projectExplorerId = 1;
    fileInfo->clusterId = filePath;
    fileInfo->parseFilePath = "";
    filePathInfos.emplace_back(fileInfo);
    ProjectExplorerInfo info = {
        1,
        "testProject",
        filePath,
        filePathInfos,
        {},
        {},
        "",
        {""},
        ""
    };
    ImportActionRequest request;
    request.params = {
        "testProject",
        {filePath},
        ProjectActionEnum::ADD_FILE,
        false
    };

    ProjectParserBin parserBin;
    parserBin.Parser({info}, request);
    BaselineInfo baselineInfo = {
        "localhost",
        "0",
        "card0",
        "",
        false
    };
    parserBin.ParserBaseline({info}, baselineInfo);

    // remove file
    BinFileGenerator::RemoveFile(fileName);
    SourceFileParser::Instance().Reset();
}