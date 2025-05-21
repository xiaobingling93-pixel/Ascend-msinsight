/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "DataBaseManager.h"
#include "TraceFileParser.h"
#include "ParserIE.h"
class ParserIETest : public ::testing::Test {};

TEST_F(ParserIETest, TestParser)
{
    class MockOpenApi : public Dic::Module::IE::ServitizationOpenApi {
    public:
        std::vector<Dic::Module::IE::TaskInfo> ComputeTaskInfo(const std::string& path)
        {
            std::vector<Dic::Module::IE::TaskInfo> tasks;
            Dic::Module::IE::TaskInfo task;
            task.fileId = "ll";
            task.filePath = "mmm";
            tasks.emplace_back(task);
            return tasks;
        }
    };
    class MockParserIE : public Dic::Module::ParserIE {
    public:
        void SetOpenApi()
        {
            servitizationOpenApi = std::make_shared<MockOpenApi>();
        }

        void Reset()
        {
            servitizationOpenApi->Reset();
        }
    };
    Dic::Module::Timeline::DataBaseManager::Instance().Clear();
    MockParserIE parserIe;
    parserIe.SetOpenApi();
    Dic::Module::Global::ProjectExplorerInfo info;
    std::vector<Dic::Module::Global::ProjectExplorerInfo> projectInfos;
    projectInfos.emplace_back(info);
    ImportActionRequest request;
    parserIe.Parser(projectInfos, request);
    parserIe.Reset();
    Dic::Module::Timeline::TraceFileParser::Instance().Reset();
    Dic::Module::Timeline::DataBaseManager::Instance().Clear();
}

TEST_F(ParserIETest, TestGetProjectType)
{
    Dic::Module::ParserIE parserIe;
    auto res = parserIe.GetProjectType({});
    EXPECT_EQ(res, ProjectTypeEnum::IE);
}