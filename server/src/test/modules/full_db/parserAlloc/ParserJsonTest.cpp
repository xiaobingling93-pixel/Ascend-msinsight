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
#include <gtest/gtest.h>
#include "../../defaultMock/MockFileReader.h"
#include "ParserJson_mock_data.h"
#include "ProjectParserJson.h"
using namespace Dic::Module;
using namespace Dic::Module::ParserJsonMock;
class ParserJsonTest : public ::testing::Test {
protected:
    inline std::string GetTestDataDir()
    {
        static std::string currPath = FileUtil::GetCurrPath();
        static auto index = currPath.find_last_of("server");
        static std::string testDataDir = currPath.substr(0, index + 1) + R"(/src/test/test_data)";
        return testDataDir;
    }
};

/**
 * 如果json文件个数为空则校验不通过
 */
TEST_F(ParserJsonTest, TestJsonFileIsEmptyThenReturnFalse)
{
    class MockParserJson : public Dic::Module::ProjectParserJson {
    public:
        void SetIFileReader(std::unique_ptr<IFileReader> fileReaderPtr)
        {
            fileReader = std::move(fileReaderPtr);
        }
        bool CheckParseFileInfoSizeTest(const std::shared_ptr<Global::ParseFileInfo> parseFileInfo,
                                        std::vector<std::string> &jsonFiles)
        {
            return CheckParseFileInfoSize(parseFileInfo, jsonFiles);
        }
    };
    MockParserJson parserJson;
    std::unique_ptr<IFileReader> fileReaderPtr = std::make_unique<MockFileReader>();
    parserJson.SetIFileReader(std::move(fileReaderPtr));
    auto parseFileInfo = std::make_shared<Global::ParseFileInfo>();
    std::vector<std::string> jsonFiles;
    bool result = parserJson.CheckParseFileInfoSizeTest(parseFileInfo, jsonFiles);
    EXPECT_EQ(result, false);
}

/**
 * 如果json文件个数超过100则校验不通过,小于等于100则校验通过
 */
TEST_F(ParserJsonTest, TestJsonFileCountExceed100ThenReturnFalse)
{
    class MockParserJson : public Dic::Module::ProjectParserJson {
    public:
        void SetIFileReader(std::unique_ptr<IFileReader> fileReaderPtr)
        {
            fileReader = std::move(fileReaderPtr);
        }
        bool CheckParseFileInfoSizeTest(const std::shared_ptr<Global::ParseFileInfo> parseFileInfo,
                                        std::vector<std::string> &jsonFiles)
        {
            return CheckParseFileInfoSize(parseFileInfo, jsonFiles);
        }
    };
    std::unique_ptr<IFileReader> fileReaderPtr = std::make_unique<MockFileReader>();
    MockParserJson parserJson;
    parserJson.SetIFileReader(std::move(fileReaderPtr));
    auto parseFileInfo = std::make_shared<Global::ParseFileInfo>();
    std::vector<std::string> jsonFiles;
    const uint8_t fileCount = 100;
    for (int i = 0; i < fileCount; ++i) {
        jsonFiles.emplace_back("kkkkk");
    }
    bool result = parserJson.CheckParseFileInfoSizeTest(parseFileInfo, jsonFiles);
    EXPECT_EQ(result, true);
    jsonFiles.emplace_back("lllll");
    result = parserJson.CheckParseFileInfoSizeTest(parseFileInfo, jsonFiles);
    EXPECT_EQ(result, false);
}

/**
 * 导入50个文件，如果有一个文件大小超过20G就校验不通过
 */
TEST_F(ParserJsonTest, TestCheckParseFileInfoSizeWhenOneFileIs20GThenReturnFalse)
{
    class MockParserJsonFileReader : public MockFileReader {
    public:
        virtual int64_t GetFileSize(const std::string &filePath)
        {
            return CheckParseFileInfoSizeWhenOneFileIs20GThenReturnFalseMock(filePath);
        }
    };
    class MockParserJson : public Dic::Module::ProjectParserJson {
    public:
        void SetIFileReader(std::unique_ptr<IFileReader> fileReaderPtr)
        {
            fileReader = std::move(fileReaderPtr);
        }
        bool CheckParseFileInfoSizeTest(const std::shared_ptr<Global::ParseFileInfo> parseFileInfo,
                                        std::vector<std::string> &jsonFiles)
        {
            return CheckParseFileInfoSize(parseFileInfo, jsonFiles);
        }
    };
    std::unique_ptr<IFileReader> fileReaderPtr = std::make_unique<MockParserJsonFileReader>();
    MockParserJson parserJson;
    parserJson.SetIFileReader(std::move(fileReaderPtr));
    auto parseFileInfo = std::make_shared<Global::ParseFileInfo>();
    std::vector<std::string> jsonFiles;
    const uint8_t fileCount = 50;
    for (int i = 0; i < fileCount; ++i) {
        jsonFiles.emplace_back("kkkkk");
    }
    bool result = parserJson.CheckParseFileInfoSizeTest(parseFileInfo, jsonFiles);
    EXPECT_EQ(result, false);
}

/**
 * 导入21个文件，前20个文件大小为1G，最后一个为1byte，总大小超过20G,校验不通过
 */
TEST_F(ParserJsonTest, TestCheckParseFileInfoSizeWhenTotalFileSizeExceed20GThenReturnFalse)
{
    class MockParserJsonFileReader : public MockFileReader {
    public:
        virtual int64_t GetFileSize(const std::string &filePath)
        {
            return CheckParseFileInfoSizeWhenTotalFileSizeExceed20GThenReturnFalseMock(filePath);
        }
    };
    class MockParserJson : public Dic::Module::ProjectParserJson {
    public:
        void SetIFileReader(std::unique_ptr<IFileReader> fileReaderPtr)
        {
            fileReader = std::move(fileReaderPtr);
        }
        bool CheckParseFileInfoSizeTest(const std::shared_ptr<Global::ParseFileInfo> parseFileInfo,
                                        std::vector<std::string> &jsonFiles)
        {
            return CheckParseFileInfoSize(parseFileInfo, jsonFiles);
        }
    };
    std::unique_ptr<IFileReader> fileReaderPtr = std::make_unique<MockParserJsonFileReader>();
    MockParserJson parserJson;
    parserJson.SetIFileReader(std::move(fileReaderPtr));
    auto parseFileInfo = std::make_shared<Global::ParseFileInfo>();
    std::vector<std::string> jsonFiles;
    const uint8_t fileCount = 21;
    for (int i = 0; i < fileCount; ++i) {
        jsonFiles.emplace_back("kkkkk");
    }
    bool result = parserJson.CheckParseFileInfoSizeTest(parseFileInfo, jsonFiles);
    EXPECT_EQ(result, false);
}

TEST_F(ParserJsonTest, TestCheckHasTraceJsonMemoryDataOperatorData)
{
    class MockParserJson : public Dic::Module::ProjectParserJson {
    public:
        static void CheckHasTraceJsonMemeoryDataOpDataFailTest()
        {
            Global::ProjectExplorerInfo projectExplorerInfo;
            std::vector<std::string> parseFileList = {"a.a", "invaild.csv"};
            for (const auto &parseFile: parseFileList) {
                auto parseFileInfo = std::make_shared<Global::ParseFileInfo>();
                parseFileInfo->parseFilePath = parseFile;
                projectExplorerInfo.subParseFileInfo.push_back(parseFileInfo);
            }
            auto [hasJson, hasMemory, hasOp] = CheckHasTraceJsonMemoryDataOperatorData({projectExplorerInfo});
            EXPECT_EQ(hasJson, false);
            EXPECT_EQ(hasMemory, false);
            EXPECT_EQ(hasOp, false);
        }

        static void CheckHasTraceJsonMemeoryDataOpDataSuccessTest()
        {
            Global::ProjectExplorerInfo projectExplorerInfo;
            std::vector<std::string> parseFileList = {"a.json", "memory_record.csv", "kernel_details.csv"};
            for (const auto &parseFile: parseFileList) {
                auto parseFileInfo = std::make_shared<Global::ParseFileInfo>();
                parseFileInfo->parseFilePath = parseFile;
                projectExplorerInfo.subParseFileInfo.push_back(parseFileInfo);
            }
            auto [hasJson, hasMemory, hasOp] = CheckHasTraceJsonMemoryDataOperatorData({projectExplorerInfo});
            EXPECT_EQ(hasJson, true);
            EXPECT_EQ(hasMemory, true);
            EXPECT_EQ(hasOp, true);
        }
    };

    MockParserJson::CheckHasTraceJsonMemeoryDataOpDataFailTest();
    MockParserJson::CheckHasTraceJsonMemeoryDataOpDataSuccessTest();
}

TEST_F(ParserJsonTest, BuildProjectInfoWithSingleFile)
{
    ProjectExplorerInfo projectInfo;
    std::string projectPath = GetTestDataDir() + R"(/test_rank_0/ASCEND_PROFILER_OUTPUT/trace_view.json)";
    projectInfo.fileName = projectPath;
    Dic::Module::ProjectParserJson::BuildProjectExploreInfo(projectInfo, {projectPath});
    EXPECT_EQ(projectInfo.projectFileTree.size(), 1);
    EXPECT_EQ(projectInfo.subParseFileInfo.size(), 1);
    EXPECT_EQ(projectInfo.fileInfoMap.size(), 2);  // expect has 2 elements
    auto file = projectInfo.projectFileTree[0];
    EXPECT_EQ(file->type, ParseFileType::PROJECT);
    EXPECT_EQ(file->subId, projectPath);
    EXPECT_EQ(file->subParseFile.size(), 1);
    file = file->subParseFile[0];
    EXPECT_EQ(file->type, ParseFileType::RANK);
    EXPECT_EQ(file->subId, "trace_view.json");
}

TEST_F(ParserJsonTest, BuildProjectInfoWithAscendProfilerOutputDir)
{
    ProjectExplorerInfo projectInfo;
    std::string projectPath = GetTestDataDir() + R"(/test_rank_0/ASCEND_PROFILER_OUTPUT)";
    projectInfo.fileName = projectPath;
    Dic::Module::ProjectParserJson::BuildProjectExploreInfo(projectInfo, {projectPath});
    EXPECT_EQ(projectInfo.projectFileTree.size(), 1);
    EXPECT_EQ(projectInfo.subParseFileInfo.size(), 1);
    EXPECT_EQ(projectInfo.fileInfoMap.size(), 2);  // expect has 2 elements
    auto file = projectInfo.projectFileTree[0];
    EXPECT_EQ(file->type, ParseFileType::PROJECT);
    EXPECT_EQ(file->subId, projectPath);
    EXPECT_EQ(file->subParseFile.size(), 1);
    file = file->subParseFile[0];
    EXPECT_EQ(file->type, ParseFileType::RANK);
    EXPECT_EQ(file->subId, "ASCEND_PROFILER_OUTPUT");
}

TEST_F(ParserJsonTest, GetParseFileByImportFile)
{
    ProjectParserJson parser;
    std::string msg;
    auto files = parser.GetParseFileByImportFile(
        GetTestDataDir() + R"(/test_rank_0/ASCEND_PROFILER_OUTPUT/trace_view.json)", msg);
    EXPECT_EQ(files.size(), 1);
    EXPECT_EQ(files[0], GetTestDataDir() + R"(/test_rank_0/ASCEND_PROFILER_OUTPUT/trace_view.json)");
    auto files2 = parser.GetParseFileByImportFile(GetTestDataDir() + R"(/test_rank_0/ASCEND_PROFILER_OUTPUT)", msg);
    EXPECT_EQ(files2.size(), 1);
}

TEST_F(ParserJsonTest, BuildProjectCluster)
{
    ProjectExplorerInfo projectInfo;
    ProjectParserJson::BuildProjectExploreInfo(projectInfo, {GetTestDataDir()});
    EXPECT_EQ(projectInfo.GetClusterInfos().size(), 1);
}

TEST_F(ParserJsonTest, GetDeviceIdFromMemory)
{
    std::string parseFolder = GetTestDataDir() + R"(/test_rank_0)";
    auto deviceId = ProjectParserJson::GetDeviceIdFromMemory(parseFolder);
    EXPECT_EQ(deviceId, "0");
}

TEST_F(ParserJsonTest, GetDeviceIdFromOperator)
{
    std::string parseFolder = GetTestDataDir() + R"(/test_rank_0)";
    auto deviceId = ProjectParserJson::GetDeviceIdFromKernel(parseFolder);
    EXPECT_EQ(deviceId, "");
}

TEST_F(ParserJsonTest, GetDeviceIdFromPath)
{
    std::string parseFolder = GetTestDataDir() + R"(/msprof/normal/PROF_20250620)";
    auto deviceId = ProjectParserJson::GetDeviceIdFromPath(parseFolder);
    EXPECT_EQ(deviceId, "");
}