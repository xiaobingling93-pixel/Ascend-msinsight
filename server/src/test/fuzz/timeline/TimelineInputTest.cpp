/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <fstream>
#include "FuzzDefs.h"
#include "FuzzFileUtil.h"
#include "FileUtil.h"
#include "TraceFileParser.h"

TEST(TraceFileParser, Parse)
{
    char testApi[] = "test_trace_file_parser";
    DT_FUZZ_START(0, FUZZ_RUN_TIMES, testApi, 0)
        {
            std::string outputFilePath = "./" + std::string(testApi) + ".json";;
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

                // 将变异后的文件内容传给TraceFileParser进行解析
                std::vector<std::string> pathList = { outputFilePath };
                std::string rankId = "0";
                std::string folder = "folder";
                Dic::Module::Timeline::TraceFileParser::Instance().Parse(pathList, rankId, folder);

            } else {
                std::cout << "Generate mutation file failed." << std::endl;
            }
        }
    DT_FUZZ_END()
}