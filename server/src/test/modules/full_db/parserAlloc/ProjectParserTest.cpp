/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <gtest/gtest.h>
#include <vector>
#include "ProjectParserFactory.h"

using namespace Dic::Module;
using namespace Dic::Module::Global;

class ProjectParserTest : public testing::Test {
};

TEST_F(ProjectParserTest, BaseIsParseFile)
{
    EXPECT_EQ(false, ProjectParserBase().IsParsedFile("test"));
}

TEST_F(ProjectParserTest, GetParentFolders)
{
    std::string path = "/home/user/name/profiling/cluster1/ASCEND_PROFILER_OUTPUT";
    std::string prefix = "/home/user/name/profiling";
    EXPECT_EQ(ProjectParserBase().GetParentFileList(prefix, path).size(), 1); // expect parent folders of path is  2
}

TEST_F(ProjectParserTest, GetParentFoldersPathEqualPrefix)
{
    EXPECT_EQ(ProjectParserBase().GetParentFileList("/home/user", "/home/user").empty(), true);
}

TEST_F(ProjectParserTest, GetClusterInfo)
{
    std::vector<std::string> folders = ProjectParserBase().GetParentFileList("/home/user/name/profiling",
        "/home/user/name/profiling/cluster1/host_ascend_pt/ASCEND_PROFILER_OUTPUT");
    auto [cluster, clusterPrefix] = ProjectParserBase().GetClusterInfo(folders);
    EXPECT_EQ(FileUtil::GetFileName(cluster), "cluster1");
    EXPECT_EQ(clusterPrefix, "/home/user/name/profiling/cluster1");
}

TEST_F(ProjectParserTest, GetClusterInfoEmpty)
{
    std::vector<std::string> folders = {};
    auto [cluster, clusterPrefix] = ProjectParserBase().GetClusterInfo(folders);
    EXPECT_EQ(cluster, "");
    EXPECT_EQ(clusterPrefix, "");
}
