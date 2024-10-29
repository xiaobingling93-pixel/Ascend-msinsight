/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "../../defaultMock/MockFileReader.h"
#include "ParserJson_mock_data.h"
#include "ParserJson.h"
using namespace Dic::Module;
using namespace Dic::Module::ParserJsonMock;
class ParserJsonTest : public ::testing::Test {};

/**
 * 如果json文件个数为空则校验不通过
 */
TEST_F(ParserJsonTest, TestJsonFileIsEmptyThenReturnFalse)
{
    class MockParserJson : public Dic::Module::ParserJson {
    public:
        void SetIFileReader(std::unique_ptr<IFileReader> fileReaderPtr)
        {
            fileReader = std::move(fileReaderPtr);
        }
        bool CheckParseFileInfoSizeTest(const Global::ParseFileInfo &parseFileInfo, std::vector<std::string> &jsonFiles)
        {
            return CheckParseFileInfoSize(parseFileInfo, jsonFiles);
        }
    };
    MockParserJson parserJson;
    std::unique_ptr<IFileReader> fileReaderPtr = std::make_unique<MockFileReader>();
    parserJson.SetIFileReader(std::move(fileReaderPtr));
    Global::ParseFileInfo parseFileInfo;
    std::vector<std::string> jsonFiles;
    bool result = parserJson.CheckParseFileInfoSizeTest(parseFileInfo, jsonFiles);
    EXPECT_EQ(result, false);
}

/**
 * 如果json文件个数超过100则校验不通过,小于等于100则校验通过
 */
TEST_F(ParserJsonTest, TestJsonFileCountExceed100ThenReturnFalse)
{
    class MockParserJson : public Dic::Module::ParserJson {
    public:
        void SetIFileReader(std::unique_ptr<IFileReader> fileReaderPtr)
        {
            fileReader = std::move(fileReaderPtr);
        }
        bool CheckParseFileInfoSizeTest(const Global::ParseFileInfo &parseFileInfo, std::vector<std::string> &jsonFiles)
        {
            return CheckParseFileInfoSize(parseFileInfo, jsonFiles);
        }
    };
    std::unique_ptr<IFileReader> fileReaderPtr = std::make_unique<MockFileReader>();
    MockParserJson parserJson;
    parserJson.SetIFileReader(std::move(fileReaderPtr));
    Global::ParseFileInfo parseFileInfo;
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
    class MockParserJson : public Dic::Module::ParserJson {
    public:
        void SetIFileReader(std::unique_ptr<IFileReader> fileReaderPtr)
        {
            fileReader = std::move(fileReaderPtr);
        }
        bool CheckParseFileInfoSizeTest(const Global::ParseFileInfo &parseFileInfo, std::vector<std::string> &jsonFiles)
        {
            return CheckParseFileInfoSize(parseFileInfo, jsonFiles);
        }
    };
    std::unique_ptr<IFileReader> fileReaderPtr = std::make_unique<MockParserJsonFileReader>();
    MockParserJson parserJson;
    parserJson.SetIFileReader(std::move(fileReaderPtr));
    Global::ParseFileInfo parseFileInfo;
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
    class MockParserJson : public Dic::Module::ParserJson {
    public:
        void SetIFileReader(std::unique_ptr<IFileReader> fileReaderPtr)
        {
            fileReader = std::move(fileReaderPtr);
        }
        bool CheckParseFileInfoSizeTest(const Global::ParseFileInfo &parseFileInfo, std::vector<std::string> &jsonFiles)
        {
            return CheckParseFileInfoSize(parseFileInfo, jsonFiles);
        }
    };
    std::unique_ptr<IFileReader> fileReaderPtr = std::make_unique<MockParserJsonFileReader>();
    MockParserJson parserJson;
    parserJson.SetIFileReader(std::move(fileReaderPtr));
    Global::ParseFileInfo parseFileInfo;
    std::vector<std::string> jsonFiles;
    const uint8_t fileCount = 21;
    for (int i = 0; i < fileCount; ++i) {
        jsonFiles.emplace_back("kkkkk");
    }
    bool result = parserJson.CheckParseFileInfoSizeTest(parseFileInfo, jsonFiles);
    EXPECT_EQ(result, false);
}