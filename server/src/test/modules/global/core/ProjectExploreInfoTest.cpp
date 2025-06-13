/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include <vector>
#include "SystemMemoryDatabaseDef.h"

using namespace Dic::Module::Global;

class ProjectExploreInfoTest : public testing::Test {
public:
    std::shared_ptr<ParseFileInfo> MockParseFileInfo(const std::string &subId, ParseFileType type)
    {
        auto file = std::make_shared<ParseFileInfo>();
        file->subId = subId;
        file->type = type;
        file->curDirName = FileUtil::GetFileName(subId);
        return file;
    }

    ProjectExplorerInfo BuildProjectInfo()
    {
        ProjectExplorerInfo projectInfo;
        auto project = MockParseFileInfo("project", ParseFileType::PROJECT);
        project->id = 0; // set project id:0
        projectInfo.AddSubParseFileInfo(project);
        auto cluster = MockParseFileInfo("project/cluster", ParseFileType::CLUSTER);
        cluster->id = 1; // set cluster id:1
        projectInfo.AddSubParseFileInfo(cluster);
        auto rank1 = MockParseFileInfo("project/cluster/rank1", ParseFileType::RANK);
        rank1->id = 2;  // set rank1 id: 2
        projectInfo.AddSubParseFileInfo(rank1);
        auto rank2 = MockParseFileInfo("project/cluster/rank2", ParseFileType::RANK);
        rank2->id = 3;  // set rank2 id: 3
        projectInfo.AddSubParseFileInfo("project/cluster", ParseFileType::CLUSTER, rank2);
        return projectInfo;
    }
};

TEST_F(ProjectExploreInfoTest, AddSubParseFile)
{
    auto project = BuildProjectInfo();
    EXPECT_EQ(project.subParseFileInfo.size(), 2);  // expect subParseFileInfo size equal 2
    EXPECT_EQ(project.projectFileTree.size(), 1);   // expect projectFileTree size equal 1
    auto &curLevel = project.projectFileTree[0]->subParseFile;
    EXPECT_EQ(curLevel.size(), 1);  // expect size is 1
    EXPECT_EQ(curLevel[0]->subId, "project/cluster");
    curLevel = curLevel[0]->subParseFile;
    EXPECT_EQ(curLevel.size(), 2);  // expect size  is 2, mean contains 2 rank
    EXPECT_EQ(curLevel[0]->subId, "project/cluster/rank1");
    EXPECT_EQ(curLevel[1]->subId, "project/cluster/rank2");
}

TEST_F(ProjectExploreInfoTest, GetSubParseFileInfo)
{
    auto project = BuildProjectInfo();
    auto file = project.GetSubParseFileInfo("project/cluster/rank2", ParseFileType::RANK);
    EXPECT_NE(file, nullptr);
}

TEST_F(ProjectExploreInfoTest, MergeProjectExploreInfo)
{
    ProjectExplorerInfo projectInfo;
    projectInfo.AddSubParseFileInfo(MockParseFileInfo("project", ParseFileType::PROJECT));
    projectInfo.AddSubParseFileInfo(MockParseFileInfo("project/rank12", ParseFileType::RANK));
    projectInfo.MergeProjectExploreInfo(BuildProjectInfo());
    EXPECT_EQ(projectInfo.projectFileTree.size(), 2);  // expect projectFileTree size is 2
    EXPECT_EQ(projectInfo.subParseFileInfo.size(), 3); // expect subParseFileInfo size is 3
}

TEST_F(ProjectExploreInfoTest, IsSubFile)
{
    auto parent = MockParseFileInfo("parent/data", ParseFileType::PROJECT);
    auto children = MockParseFileInfo("parent/data/children", ParseFileType::RANK);
    auto children2 = MockParseFileInfo("data", ParseFileType::RANK);
    EXPECT_EQ(ProjectExplorerInfo::IsSubFile(parent, children), true);
    EXPECT_EQ(ProjectExplorerInfo::IsSubFile(parent, children2), false);
}

TEST_F(ProjectExploreInfoTest, DeleteSingleFileTree)
{
    auto project = BuildProjectInfo();
    auto deletedFile = MockParseFileInfo("project/cluster/rank1", ParseFileType::RANK);
    deletedFile->id = 2;  //  set id=2
    auto ids = project.GetFileIdsToDelete(deletedFile);
    EXPECT_EQ(project.subParseFileInfo.size(), 2); // expect subParseFileInfo size is 2
    EXPECT_EQ(project.fileInfoMap.size(), 4);  // expect fileInfoMap has 4 element
    EXPECT_EQ(ids.size(), 1); // expect size 1
    EXPECT_EQ(ids[0], 2); // expect first id is 2
}

TEST_F(ProjectExploreInfoTest, DeleteClusterFileTree)
{
    auto project = BuildProjectInfo();
    auto deletedFile = MockParseFileInfo("project/cluster", ParseFileType::RANK);
    deletedFile->id = 1;
    auto ids = project.GetFileIdsToDelete(deletedFile);
    EXPECT_EQ(project.subParseFileInfo.size(), 2); // expect subParseFileInfo size is 2
    EXPECT_EQ(ids.size(), 4); // expect ids size is 4
}
