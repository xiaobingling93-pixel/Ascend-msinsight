/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "BaselineManager.h"
#include "ProjectExplorerManager.h"
#include "DataBaseManager.h"
#include "ParserStatusManager.h"

using namespace Dic::Module::Global;
class BaselineManagerTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        std::string systemDbPath = currPath.substr(0, index + 1) + R"(/src/test/test_data/)";
        ProjectExplorerManager::Instance().InitSystemMemoryDbPath(systemDbPath);
        InitProjectExplorerData();
    }

    static void TearDownTestCase()
    {
        ClearProjectExplorerData();
    }
protected:
    inline static std::string currPath = Dic::FileUtil::GetCurrPath();
    inline static int index = currPath.find_last_of("server");
    inline static int retry = 5;
    static ProjectExplorerInfo CreateProjectData(const std::string &projectName, const std::string &fileName,
                                                 const std::string &importType, Dic::ProjectTypeEnum projectType,
                                                 const std::vector<std::string> parseFileList)
    {
        ProjectExplorerInfo info;
        info.projectName = projectName;
        info.fileName = fileName;
        info.importType = importType;
        info.projectType = static_cast<int64_t>(projectType);
        for (const auto &item: parseFileList) {
            ParseFileInfo parseFileInfo;
            parseFileInfo.parseFilePath = item;
            info.parseFilePathInfos.push_back(parseFileInfo);
        }
        return info;
    }
    static void InitProjectExplorerData()
    {
        std::string filePathText = currPath.substr(0, index + 1) +
            R"(/src/test/test_data/test_rank_0/ASCEND_PROFILER_OUTPUT)";
        std::string filePathDb = currPath.substr(0, index + 1) +
            R"(/src/test/test_data/full_db/ascend_pytorch_profiler.db)";
        std::vector<ProjectExplorerInfo> infos;
        std::vector<std::string> parseFileList {filePathText};
        ProjectExplorerInfo info = CreateProjectData("testProject", "projectFilePath",
                                                     "import", Dic::ProjectTypeEnum::TEXT_CLUSTER, parseFileList);
        infos.push_back(info);
        ProjectExplorerManager::Instance().SaveProjectExplorer(infos, false);

        std::vector<ProjectExplorerInfo> dbInfos;
        std::vector<std::string> parseDbFileList {filePathDb};
        ProjectExplorerInfo dbInfo = CreateProjectData("testProjectDb", "projectFilePathDb",
                                                       "import", Dic::ProjectTypeEnum::DB, parseDbFileList);
        dbInfos.push_back(dbInfo);
        ProjectExplorerManager::Instance().SaveProjectExplorer(dbInfos, false);
    }

    static void ClearProjectExplorerData()
    {
        ProjectExplorerManager::Instance().DeleteProjectAndFilePath("testProject",
                                                                    std::vector<std::string>());
        ProjectExplorerManager::Instance().DeleteProjectAndFilePath("testProjectDb",
                                                                    std::vector<std::string>());
    }
};

// 测试text数据baseline设置正常情况
TEST_F(BaselineManagerTest, testText)
{
    std::string errorMsg;
    std::string rankId;
    std::string filePathText = currPath.substr(0, index + 1) +
        R"(/src/test/test_data/test_rank_0/ASCEND_PROFILER_OUTPUT)";
    BaselineManager::Instance().InitBaselineData("testProject", filePathText, errorMsg, rankId);
    std::string notFinishTask = "";
    int index = 0;
    while (index < retry && !Dic::Protocol::ParserStatusManager::Instance().IsAllFinished(notFinishTask)) {
        const int sleepTime = 2000;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        index++;
    }
    EXPECT_EQ(BaselineManager::Instance().GetBaselineId(), rankId);
}

// 测试db数据baseline设置正常情况
TEST_F(BaselineManagerTest, testDb)
{
    std::string errorMsg;
    std::string rankId;
    std::string filePathDb = currPath.substr(0, index + 1) +
        R"(/src/test/test_data/full_db/ascend_pytorch_profiler.db)";
    BaselineManager::Instance().InitBaselineData("testProjectDb", filePathDb, errorMsg, rankId);
    std::string notFinishTask = "";
    int index = 0;
    while (index < retry && !Dic::Protocol::ParserStatusManager::Instance().IsAllFinished(notFinishTask)) {
        const int sleepTime = 2000;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        index++;
    }
    EXPECT_EQ(BaselineManager::Instance().GetBaselineId(), rankId);
}

// 测试db不存在的场景
TEST_F(BaselineManagerTest, testFileNotExist)
{
    std::string errorMsg;
    std::string rankId;
    std::string filePathDb = "noData";
    BaselineManager::Instance().InitBaselineData("testProjectDb", filePathDb, errorMsg, rankId);
    EXPECT_EQ(BaselineManager::Instance().GetBaselineId(), "baseline-1");
    EXPECT_EQ(rankId, "");
}