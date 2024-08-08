/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "FileUtil.h"
#include "ProjectExplorerManager.h"
#include "GlobalDefs.h"

using namespace Dic::Module::Global;
class ProjectExplorerManagerTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        std::string systemDbPath = currPath.substr(0, index + 1) + R"(/src/test/test_data/)";
        ProjectExplorerManager::Instance().InitSystemMemoryDbPath(systemDbPath);
    }

    static void TearDownTestCase() {}

protected:
    static std::string curServerPath;
    void InitProjectExplorerData()
    {
        ProjectExplorerManager::Instance().SaveProjectExplorer("testProject", "projectFilePath",
            Dic::ProjectTypeEnum::CLUSTER, "import", std::vector<std::string>());
    }

    void ClearProjectExplorerData()
    {
        ProjectExplorerManager::Instance().DeleteProjectAndFilePath("testProject",
                                                                    std::vector<std::string>());
        ProjectExplorerManager::Instance().DeleteProjectAndFilePath("newTestProject",
                                                                    std::vector<std::string>());
    }
};

// 更新项目名成功
TEST_F(ProjectExplorerManagerTest, UpdateProjectNameSuccess)
{
    InitProjectExplorerData();
    std::string projectName = "testProject";
    std::string newProjectName = "newTestProject";
    bool updateRes = ProjectExplorerManager::Instance().UpdateProjectName(projectName, newProjectName);
    EXPECT_EQ(updateRes, true);
    std::vector<ProjectExplorerInfo> res = ProjectExplorerManager::Instance()
            .QueryProjectExplorer(newProjectName, std::vector<std::string>());
    EXPECT_EQ(res.size(), 1);
    EXPECT_EQ(res[0].importType, "import");
    EXPECT_EQ(res[0].fileName, "projectFilePath");
    EXPECT_EQ(static_cast<Dic::ProjectTypeEnum>(res[0].projectType), Dic::ProjectTypeEnum::CLUSTER);
    ClearProjectExplorerData();
}

// 同一个项目下导入文件，存在冲突，并覆盖文件
TEST_F(ProjectExplorerManagerTest, CheckProjectConflictAndCoverData)
{
    std::string projectName = "testProject";
    InitProjectExplorerData();
    std::vector<std::string> filePathList;
    Dic::Protocol::ProjectCheckBody body;
    std::string currPath = Dic::FileUtil::GetCurrPath();
    int index = currPath.find_last_of("server");
    std::string filePath =
            currPath.substr(0, index + 1) + "/src/test/test_data/test_rank_1/ASCEND_PROFILER_OUTPUT";
    filePathList.push_back(filePath);
    bool result = ProjectExplorerManager::Instance().CheckProjectConflict(projectName, filePathList);
    EXPECT_EQ(result, true);
    bool res = ProjectExplorerManager::Instance().SaveProjectExplorer(projectName, filePathList[0],
        Dic::ProjectTypeEnum::TRACE, "import", std::vector<std::string>());
    EXPECT_EQ(res, true);
    std::vector<ProjectExplorerInfo> queryRes = ProjectExplorerManager::Instance()
            .QueryProjectExplorer(projectName, std::vector<std::string>());
    EXPECT_EQ(queryRes.size(), 1);
    EXPECT_EQ(queryRes[0].importType, "import");
    EXPECT_EQ(queryRes[0].fileName, filePath);
    EXPECT_EQ(static_cast<Dic::ProjectTypeEnum>(queryRes[0].projectType), Dic::ProjectTypeEnum::TRACE);
    ClearProjectExplorerData();
}