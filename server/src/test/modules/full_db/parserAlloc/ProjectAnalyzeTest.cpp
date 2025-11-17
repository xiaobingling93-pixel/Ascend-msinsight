/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include "ProjectAnalyze.h"
#include "ProjectParserBin.h"
#include "ProjectParserDb.h"
#include "ProjectParserJson.h"

using namespace Dic::Module;
using namespace Dic;

class ProjectAnalyzeTest : public testing::Test {
};

TEST_F(ProjectAnalyzeTest, ProjectAnalyzeRegister)
{
    ProjectAnalyzeRegister<ProjectParserBin> reg(ParserType::BIN);
    ProjectAnalyzeRegister<ProjectParserDb> reg2(ParserType::DB);
    ProjectAnalyzeRegister<ProjectParserJson> reg3(ParserType::JSON);
    std::vector<ParserType> types = {ParserType::BIN, ParserType::DB, ParserType::JSON};
    for (auto type: types) {
        ProjectExplorerInfo projectInfo;
        ProjectAnalyze::Instance().ProjectExportInfoBuild(type, {"test"}, projectInfo);
    }
}