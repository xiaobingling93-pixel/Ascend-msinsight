/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <string>
#include <fstream>
#include <gtest/gtest.h>
#include "ComputeFuzz.h"
#include "SourceFileParser.h"
#include "FileUtil.h"
#include "JsonUtil.h"

using namespace Dic;
using namespace Module;
using namespace Source;

TEST(TestInputFiles, ApiFiles)
{
    char testApi[] = "test_api_input_file";
    DT_FUZZ_START(0, g_fuzzRunTime, testApi, 0)
    {
        printf("fuzzseed is %d \n", fuzzSeed);
        printf("fuzzi is %d \n", fuzzi);
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
        EXPECT_EQ(true, parser.CheckOperatorBinary(binFilePath));
        std::string fileId = "testFile" + std::to_string(fuzzi);
        printf("fileId is %s \n", fileId.c_str());
        EXPECT_TRUE(parser.Parse(std::vector<std::string>(), fileId, binFilePath));
    }
    DT_FUZZ_END()
}

TEST(TestInputFile, CheckJsonFormat)
{
    char testApi[] = "test_json_fromat";
    DT_FUZZ_START(0, g_fuzzRunTime, testApi, 0)
    {
    char* content = DT_SetGetStringNum(&g_Element[0], 2, UINT64_MAX, "a");

    printf("*******json str is %s", content);

    char* contentArray = DT_SetGetStringNum(&g_Element[1], 2, UINT64_MAX, "b");

    printf("*******json array is %s", contentArray);
    std::string fullStr = std::string("{\"args\":\"") + content + "\",\"array\":[\"" + contentArray + "\"]}";

    printf("\n Full Str is %s \n", fullStr.c_str());

    document_t json;
    EXPECT_EQ(json.Parse(fullStr.c_str()).HasParseError(), false);
    ASSERT_EQ(json.IsObject(), true);
    printf("*********** IS OBJECT\n");
    EXPECT_FALSE(JsonUtil::IsJsonArray(json, "args"));
    EXPECT_TRUE(JsonUtil::IsJsonArray(json, "array"));
    }
    DT_FUZZ_END()
}

TEST(TestInputFile, JsonParse)
{
    char testApi[] = "test_json_parse";
    DT_FUZZ_START(0, g_fuzzRunTime, testApi, 0)
    {
        printf("fuzz run %d start \n", fuzzi);

        std::string errMsg;
        char* content = DT_SetGetStringNum(&g_Element[1], 2, UINT64_MAX, "x");
        std::string contentStr(content);
        std::string jsonStr = std::string("{\"cat\":") + contentStr + "}";
        const std::optional<document_t> &json = JsonUtil::TryParse(jsonStr, errMsg);
        printf("\n jsonStr is %s \n", jsonStr.c_str());
        EXPECT_TRUE(json.has_value());
        printf("\n ERRMSG IS %s \n", errMsg.substr(0, 13).c_str());

        printf("fuzz run %d end \n", fuzzi);
    }
    DT_FUZZ_END()
}

TEST(TestInputFile, JsonParseErr)
{
    char testApi[] = "test_json_parse_err";
    DT_FUZZ_START(0, g_fuzzRunTime, testApi, 0)
                {
                    printf("fuzz run %d start \n", fuzzi);
                    std::string errMsg;
                    char* content = DT_SetGetStringNum(&g_Element[1], 2, UINT64_MAX, "x");
                    std::string contentStr(content);
                    std::string jsonStr = std::string(R"({"cat":")") + contentStr + "\"}, ";
                    JsonUtil::TryParse(jsonStr, errMsg);
                    printf("\n jsonStr is %s \n", jsonStr.c_str());
                    EXPECT_EQ(errMsg.substr(0, 13), "Error code:3.");
                    printf("fuzz run %d end \n", fuzzi);
                }
    DT_FUZZ_END()
}


TEST(TestInputFilesPath, CheckPath)
{
    char testApi[] = "test_api_input_file_path";
    DT_FUZZ_START(0, g_fuzzRunTime, testApi, 0)
                {
                    char* filePath = DTSetGetString(&g_Element[0], 5, PATH_MAX, "path", "LENTH");
                    EXPECT_TRUE(Dic::FileUtil::CheckFilePathLength(filePath));
                    printf("file path is %s\n", filePath);
                    char* realPath = "execute_fuzz_test.sh";
                    int ret = symlink(realPath, filePath);
                    if (ret) {
                        EXPECT_TRUE(Dic::FileUtil::IsSoftLink(filePath));
                        unlink(filePath);
                        printf("unlink\n");
                    } else {
                        perror("symlink");
                    }
                }
    DT_FUZZ_END()
}