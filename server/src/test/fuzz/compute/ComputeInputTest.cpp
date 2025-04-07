/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <string>
#include <fstream>
#include <gtest/gtest.h>
#include "../FuzzDefs.h"
#include "ComputeFuzz.h"
#include "SourceFileParser.h"
#include "JsonUtil.h"

using namespace Dic;
using namespace Module;
using namespace Source;

TEST(TestInputFiles, ApiFiles)
{
    char testApi[] = "test_api_input_file";
    DT_FUZZ_START(0, FUZZ_RUN_TIMES, testApi, 0)
    {
        std::ofstream apiFile("./visualize_data.bin", std::ios::binary|std::ios::trunc);
        BinaryBlockHeader binaryBlockHeader;
        BinaryBlockHeader4File binaryBlockHeader4File;
        binaryBlockHeader.type = 1;

        char* filePath = DT_SetGetString(&g_Element[0], 6, 4096, "a.cpp");
        char* fileContent = DT_SetGetStringNum(&g_Element[1], 2, UINT64_MAX, "a");
        int fileSize = DT_GET_MutatedValueLen(&g_Element[1]);
        std::string fileContentStr(fileContent);

        GetFilePadding(binaryBlockHeader, fileSize, fileContentStr);
        binaryBlockHeader.contentSize = fileSize + binaryBlockHeader.padding;
        binaryBlockHeader4File.binaryBlockHeader = binaryBlockHeader;
        for (int i = 0; i < sizeof(binaryBlockHeader4File.filePath); ++i) {
            binaryBlockHeader4File.filePath[i] = filePath[i];
        }

        apiFile.write(reinterpret_cast<const char *>(&binaryBlockHeader4File), sizeof(binaryBlockHeader4File));
        apiFile.write(fileContentStr.c_str(), fileSize);
        apiFile.close();

        std::string binFilePath = "./visualize_data.bin";
        Module::Source::SourceFileParser &parser = Dic::Module::Source::SourceFileParser::Instance();
        std::string errMsg;
        EXPECT_EQ(true, parser.CheckOperatorBinary(binFilePath, errMsg));
        std::string fileId = "testFile" + std::to_string(fuzzi);
        EXPECT_TRUE(parser.Parse(std::vector<std::string>(), fileId, binFilePath));
    }
    DT_FUZZ_END()
}

TEST(TestInputFile, CheckJsonFormat)
{
    char testApi[] = "test_json_format";
    DT_FUZZ_START(0, FUZZ_RUN_TIMES, testApi, 0)
    {
    char* content = DT_SetGetStringNum(&g_Element[0], 2, UINT64_MAX, "a");
    char* contentArray = DT_SetGetStringNum(&g_Element[1], 2, UINT64_MAX, "b");
    std::string fullStr = std::string("{\"args\":\"") + content + "\",\"array\":[\"" + contentArray + "\"]}";
    document_t json;
    EXPECT_EQ(json.Parse(fullStr.c_str()).HasParseError(), false);
    ASSERT_EQ(json.IsObject(), true);
    EXPECT_FALSE(JsonUtil::IsJsonArray(json, "args"));
    EXPECT_TRUE(JsonUtil::IsJsonArray(json, "array"));
    }
    DT_FUZZ_END()
}

TEST(TestInputFile, JsonParse)
{
    char testApi[] = "test_json_parse";
    DT_FUZZ_START(0, FUZZ_RUN_TIMES, testApi, 0)
    {
        std::string errMsg;
        char* content = DT_SetGetStringNum(&g_Element[0], 2, UINT64_MAX, "0");
        std::string contentStr(content);
        std::string jsonStr = std::string("{\"cat\":") + contentStr + "}";
        const std::optional<document_t> &json = JsonUtil::TryParse(jsonStr, errMsg);
        EXPECT_TRUE(json.has_value());
    }
    DT_FUZZ_END()
}

TEST(TestInputFile, JsonParseErr)
{
    char testApi[] = "test_json_parse_err";
    DT_FUZZ_START(0, FUZZ_RUN_TIMES, testApi, 0)
                {
                    std::string errMsg;
                    char* content = DT_SetGetStringNum(&g_Element[0], 2, UINT64_MAX, "0");
                    std::string contentStr(content);
                    std::string jsonStr = std::string(R"({"cat":")") + contentStr + "\"}, ";
                    JsonUtil::TryParse(jsonStr, errMsg);
                    EXPECT_EQ(errMsg.substr(0, 13), "Error code:2.");
                }
    DT_FUZZ_END()
}


TEST(TestCompute, SourceFileParserCheckPath)
{
    char testApi[] = "test_source_file_parser_check_path";
    DT_FUZZ_START(0, FUZZ_RUN_TIMES, testApi, 0)
                {
                    char* filePath = DTSetGetString(&g_Element[0], 5, PATH_MAX, "path", "selectedFile");
                    std::vector<std::string> filePaths = {};
                    std::string fileId(filePath);
                    SourceFileParser::Instance().Parse(filePaths, fileId, filePath);
                }
    DT_FUZZ_END()
}

TEST(TestCompute, SourceFileParserParseRandomContent)
{
    char testApi[] = "test_source_file_parser_parse_random_content";
    DT_FUZZ_START(0, FUZZ_RUN_TIMES, testApi, 0)
                {
                    std::string binFilePath = "./test_source_file_parser_parse_random_content_visualize_data.bin";
                    std::ofstream binFile(binFilePath, std::ios::binary | std::ios::trunc);

                    char* fileContent = DT_SetGetBlob(&g_Element[0], 2, UINT32_MAX, "a");
                    int fileSize = DT_GET_MutatedValueLen(&g_Element[0]);
                    binFile.write(fileContent, fileSize);
                    binFile.close();

                    Module::Source::SourceFileParser &parser = Dic::Module::Source::SourceFileParser::Instance();
                    parser.Parse(std::vector<std::string>(), binFilePath, binFilePath);
                }
    DT_FUZZ_END()
}