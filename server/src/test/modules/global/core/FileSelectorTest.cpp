/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include "BaselineManagerService.h"
#include "ParserStatusManager.h"
#include "FileSelector.h"
#include "GlobalProtocolResponse.h"
#include "ProtocolDefs.h"
#include "FileUtil.h"

using namespace Dic::Module::Global;
using namespace Dic::Protocol;
class FileSelectorTest : public ::testing::Test {
protected:
    inline static std::string currPath = Dic::FileUtil::GetCurrPath();
    inline static int index = currPath.find_last_of("server");
};

TEST_F(FileSelectorTest, TestPathNotExist)
{
    std::string path = "xxx";
    std::vector<std::unique_ptr<Folder>> childrenFolders;
    std::vector<std::unique_ptr<File>> childrenFiles;
    bool exist = true;
    FileSelector::GetFoldersAndFiles(path, childrenFolders, childrenFiles, exist);
    EXPECT_FALSE(exist);
    ASSERT_TRUE(childrenFolders.empty());
    ASSERT_TRUE(childrenFiles.empty());
}

TEST_F(FileSelectorTest, TestContainFolder)
{
    std::string path =  currPath.substr(0, index + 1) + R"(\src\test\test_data\msprof)";
    std::vector<std::unique_ptr<Folder>> childrenFolders;
    std::vector<std::unique_ptr<Folder>> realChildrenFolders;
    std::vector<std::unique_ptr<File>> childrenFiles;
    bool exist = false;
    std::vector<std::string> folders = {"normal", "slice"};
    for (const auto &folder : folders) {
        auto folderPtr = std::make_unique<Folder>();
        folderPtr->name = folder;
        folderPtr->path = Dic::FileUtil::SplicePath(path, folder);
        realChildrenFolders.emplace_back(std::move(folderPtr));
    }
    FileSelector::GetFoldersAndFiles(path, childrenFolders, childrenFiles, exist);
    EXPECT_TRUE(exist);
    for (auto i = 0; i < childrenFolders.size(); i++) {
        ASSERT_EQ(childrenFolders[i]->name, realChildrenFolders[i]->name);
    }
    ASSERT_TRUE(childrenFiles.empty());
}

TEST_F(FileSelectorTest, TestContainFiles)
{
    std::string path =  currPath.substr(0, index + 1) + R"(\src\test\test_data\full_db)";
    std::vector<std::unique_ptr<Folder>> childrenFolders;
    std::vector<std::unique_ptr<File>> childrenFiles;
    std::vector<std::unique_ptr<File>> realChildrenFiles;
    bool exist = false;
    std::vector<std::string> files = {"ascend_pytorch_profiler.db", "cluster_analysis.db", "msprof_0.db"};
    for (const auto &file : files) {
        std::string tmpPath = Dic::FileUtil::SplicePath(path, file);
        realChildrenFiles.emplace_back(std::make_unique<File>(file, tmpPath));
    }
    FileSelector::GetFoldersAndFiles(path, childrenFolders, childrenFiles, exist);
    EXPECT_TRUE(exist);
    for (int i = 0; i < realChildrenFiles.size(); i++) {
        ASSERT_EQ(realChildrenFiles[0]->name, childrenFiles[i]->name);
    }
    ASSERT_TRUE(childrenFolders.empty());
}

TEST_F(FileSelectorTest, TestContainFolderandFiles)
{
    std::string path =  currPath.substr(0, index + 1) + R"(\src\test\test_data\test_rank_0)";
    std::vector<std::unique_ptr<Folder>> childrenFolders;
    std::vector<std::unique_ptr<Folder>> realChildrenFolders;
    std::vector<std::unique_ptr<File>> childrenFiles;
    std::vector<std::unique_ptr<File>> realChildrenFiles;
    bool exist = false;
    std::vector<std::string> folders = {"ASCEND_PROFILER_OUTPUT"};
    for (const auto &folder : folders) {
        auto folderPtr = std::make_unique<Folder>();
        folderPtr->name = folder;
        folderPtr->path = Dic::FileUtil::SplicePath(path, folder);
        realChildrenFolders.emplace_back(std::move(folderPtr));
    }
    std::vector<std::string> files = {"profiler_info_0.json"};
    for (const auto &file : files) {
        std::string tmpPath = Dic::FileUtil::SplicePath(path, file);
        realChildrenFiles.emplace_back(std::make_unique<File>(file, tmpPath));
    }
    FileSelector::GetFoldersAndFiles(path, childrenFolders, childrenFiles, exist);
    EXPECT_TRUE(exist);
    for (auto i = 0; i < childrenFolders.size(); i++) {
        ASSERT_EQ(childrenFolders[i]->name, realChildrenFolders[i]->name);
    }
    for (int i = 0; i < realChildrenFiles.size(); i++) {
        ASSERT_EQ(realChildrenFiles[0]->name, childrenFiles[i]->name);
    }
}