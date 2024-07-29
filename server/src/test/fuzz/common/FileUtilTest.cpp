/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <fstream>
#include "FuzzDefs.h"
#include "FuzzFileUtil.h"
#include "FileUtil.h"
#include "JsonUtil.h"

using namespace Dic;

TEST(TestFileUtil, IsAbsolutePath)
{
    char testApi[] = "test_file_util_is_absolute_path";
    DT_FUZZ_START(0, FUZZ_RUN_TIMES, testApi, 0)
        {
            char* filePath = DT_SetGetString(&g_Element[0], 5, UINT32_MAX, "path");
            FileUtil::IsAbsolutePath(filePath);
        }
    DT_FUZZ_END()
}

TEST(TestFileUtil, CopyFileByPath)
{
    char testApi[] = "test_file_util_copy_file_by_path";
    DT_FUZZ_START(0, FUZZ_RUN_TIMES, testApi, 0)
                {
                    std::string binFilePath = "./test_file_util_copy_file_by_path.bin";
                    std::ofstream binFile(binFilePath, std::ios::binary | std::ios::trunc);

                    char* fileContent = DT_SetGetBlob(&g_Element[0], 2, UINT32_MAX, "a");
                    int fileSize = DT_GET_MutatedValueLen(&g_Element[0]);
                    binFile.write(fileContent, fileSize);
                    binFile.close();

                    std::string copyPath = "./test_file_util_copy_file_by_path.copy";
                    FileUtil::CopyFileByPath(binFilePath, copyPath);
                }
    DT_FUZZ_END()
}

TEST(TestFileUtil, PathPreprocess)
{
    char testApi[] = "test_file_util_path_preprocess";
    DT_FUZZ_START(0, FUZZ_RUN_TIMES, testApi, 0)
                {
                    char* filePath = DT_SetGetString(&g_Element[0], 4, PATH_MAX, "./a");
                    FileUtil::PathPreprocess(filePath);
                }
    DT_FUZZ_END()
}

TEST(JsonUtil, TryParse)
{
    char testApi[] = "test_json_util_try_parse";
    DT_FUZZ_START(0, FUZZ_RUN_TIMES, testApi, 0)
                {
                    std::string outputFilePath = "./" + std::string(testApi) + ".json";
                    std::string inputFilePath = "./test_data/trace_view_tiny.json";

                    char* fileContent = nullptr;
                    int fileContentSize = 0;
                    if (GenerateFileMutation(inputFilePath, &fileContent, fileContentSize) == 0) {
                        std::ofstream outputFile(outputFilePath, std::ios::trunc);
                        if (!outputFile) {
                            std::cout << "open output file failed: " << outputFilePath << std::endl;
                            return;
                        }
                        outputFile.write(fileContent, fileContentSize);
                        outputFile.close();
                        std::string errMsg;
                        JsonUtil::TryParse(fileContent, errMsg);
                    } else {
                        std::cout << "Generate mutation file failed." << std::endl;
                    }
                }
    DT_FUZZ_END()
}
