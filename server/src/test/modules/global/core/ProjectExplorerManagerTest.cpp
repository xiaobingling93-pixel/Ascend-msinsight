/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "FileUtil.h"
#include "ProjectExplorerManager.h"
#include "GlobalDefs.h"
#include "ProjectParserJson.h"

const int64_t NUMBER_ZERO = 0;
const int64_t NUMBER_THREE = 3;
const int64_t NUMBER_FIVE = 5;
const int64_t NUMBER_SIX = 6;
using namespace Dic::Module::Global;
class ProjectExplorerManagerTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        std::string systemDbPath = currPath.substr(0, index + 1) + R"(/src/test/test_data/)";
        ProjectExplorerManager::Instance().InitSystemMemoryDbPath(systemDbPath);
    }

    static void TearDownTestSuite() {}

protected:
    inline static std::string currPath = FileUtil::GetCurrPath();
    inline static auto index = currPath.find_last_of("server");
    inline static std::string curServerPath = currPath.substr(0, index + 1) + R"(src/test/test_data)";
    ProjectExplorerInfo CreateProjectData(const std::string &projectName, const std::string &fileName,
                                          const std::string &importType, Dic::ProjectTypeEnum projectType,
                                          const std::vector<std::string> dbPath)
    {
        ProjectExplorerInfo info;
        info.projectName = projectName;
        info.fileName = fileName;
        info.importType = importType;
        info.projectType = static_cast<int64_t>(projectType);
        info.dbPath = dbPath;
        info.accessTime = "2000-01-01 00:00:00.000";
        auto parseFileInfo = std::make_shared<ParseFileInfo>();
        parseFileInfo->parseFilePath = "test";
        parseFileInfo->subId = "test";
        parseFileInfo->type = ParseFileType::RANK;
        info.AddSubParseFileInfo(parseFileInfo);
        return info;
    }
    void InitProjectExplorerData()
    {
        std::vector<ProjectExplorerInfo> infos;
        ProjectExplorerInfo info = CreateProjectData("testProject", "projectFilePath",
                                                     "import", Dic::ProjectTypeEnum::TEXT_CLUSTER,
                                                     std::vector<std::string>());
        infos.push_back(info);
        std::for_each(infos.begin(), infos.end(), [](const auto& item) {
            ProjectExplorerManager::Instance().SaveProjectExplorer(item, false);
        });
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
    EXPECT_EQ(static_cast<Dic::ProjectTypeEnum>(res[0].projectType), Dic::ProjectTypeEnum::TEXT_CLUSTER);
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
            currPath.substr(0, index + 1) + "/src/test/test_data/data.bin";
    filePathList.push_back(filePath);
    Dic::Protocol::ProjectErrorType result = ProjectExplorerManager::Instance().CheckProjectConflict(projectName,
                                                                                                     filePathList[0]);
    bool isConflict = (result == Dic::Protocol::ProjectErrorType::PROJECT_NAME_CONFLICT);
    EXPECT_EQ(isConflict, true);
    std::vector<ProjectExplorerInfo> infos;
    ProjectExplorerInfo info = CreateProjectData(projectName, filePathList[0], "import",
                                                 Dic::ProjectTypeEnum::BIN, std::vector<std::string>());
    infos.push_back(info);
    std::for_each(infos.begin(), infos.end(), [](const auto& item) {
        bool res = ProjectExplorerManager::Instance().SaveProjectExplorer(item, false);
        EXPECT_EQ(res, true);
    });
    std::vector<ProjectExplorerInfo> queryRes = ProjectExplorerManager::Instance()
            .QueryProjectExplorer(projectName, std::vector<std::string>());
    EXPECT_EQ(queryRes.size(), 2); // expect size 2
    EXPECT_EQ(queryRes[0].importType, "import");
    EXPECT_EQ(queryRes[0].fileName, filePath);
    EXPECT_EQ(static_cast<Dic::ProjectTypeEnum>(queryRes[0].projectType), Dic::ProjectTypeEnum::BIN);
    ClearProjectExplorerData();
}

// 清空项目内容
TEST_F(ProjectExplorerManagerTest, ClearProjectExplorerSuccess)
{
    ProjectExplorerManager::Instance().ClearProjectExplorer(std::vector<std::string>{});
    InitProjectExplorerData();
    bool result = ProjectExplorerManager::Instance().ClearProjectExplorer(std::vector<std::string>{});
    EXPECT_EQ(result, true);
    std::vector<ProjectExplorerInfo> queryRes = ProjectExplorerManager::Instance()
            .QueryProjectExplorer("", std::vector<std::string>());
    EXPECT_EQ(queryRes.size(), 0);
}

TEST_F(ProjectExplorerManagerTest, GetProjectTypeWithOnlyMulitTextType)
{
    std::vector<ProjectExplorerInfo> projectInfoList;
    ProjectExplorerInfo info1;
    info1.projectType = NUMBER_FIVE;
    ProjectExplorerInfo info2;
    info2.projectType = NUMBER_THREE;
    projectInfoList.push_back(info1);
    projectInfoList.push_back(info2);
    Dic::ProjectTypeEnum type = ProjectExplorerManager::GetProjectType(projectInfoList);
    EXPECT_EQ(type, Dic::ProjectTypeEnum::TEXT_CLUSTER);
}

TEST_F(ProjectExplorerManagerTest, GetProjectTypeWithOnlyMulitDbType)
{
    std::vector<ProjectExplorerInfo> projectInfoList;
    ProjectExplorerInfo info1;
    info1.projectType = NUMBER_ZERO;
    ProjectExplorerInfo info2;
    info2.projectType = NUMBER_SIX;
    projectInfoList.push_back(info1);
    projectInfoList.push_back(info2);
    Dic::ProjectTypeEnum type = ProjectExplorerManager::GetProjectType(projectInfoList);
    EXPECT_EQ(type, Dic::ProjectTypeEnum::DB_CLUSTER);
}

TEST_F(ProjectExplorerManagerTest, DeleteFileNode)
{
    std::string importPath =  + R"(/test_rank_1)";
    ProjectExplorerInfo projectInfo;
    projectInfo.fileName = importPath;
    projectInfo.projectName = "testDeleteFile";
    projectInfo.id = 1;
    std::string parsedFilePath = importPath + R"(ASCEND_PROFILER_OUTPUT)";
    Dic::Module::ProjectParserJson::BuildProjectExploreInfo(projectInfo, {parsedFilePath});
    ProjectExplorerManager::Instance().SaveProjectExplorer(projectInfo, false);
    ProjectExplorerManager::Instance().DeleteProjectAndFilePath("testDeleteFile", {parsedFilePath});
    auto result = ProjectExplorerManager::Instance().QueryProjectExplorer("testDeleteFile", {});
    EXPECT_EQ(result.size(), 0);
}