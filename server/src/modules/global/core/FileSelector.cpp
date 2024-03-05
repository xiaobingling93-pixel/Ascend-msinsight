/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "FileUtil.h"
#include "FileSelector.h"

namespace Dic {
namespace Module {
namespace Global {
using namespace Protocol;
void FileSelector::GetFoldersAndFiles(const std::string &path,
                                      std::vector<std::unique_ptr<Protocol::Folder>> &childrenFolders,
                                      std::vector<std::unique_ptr<Protocol::File>> &childrenFiles,
                                      bool &exist)
{
    std::vector<std::string> folders;
    std::vector<std::string> files;
    // 处理中文路径
    std::string tempPath = FileUtil::PathPreprocess(path);
    // 防止目录遍历攻击
    std::string filepath = FileUtil::GetRealPath(tempPath);
    if (path.empty()) {
        folders = FileUtil::GetDiskInfo();
        exist = false;
    } else if (tempPath != filepath) {
        exist = false;
        return;
    } else if (tempPath == filepath) {
        exist = FileUtil::FindFolders(path, folders, files) || FileUtil::CheckFilePath(tempPath);
    }
    for (const auto &folder : folders) {
        auto folderPtr = std::make_unique<Folder>();
        folderPtr->name = folder;
        folderPtr->path = FileUtil::SplicePath(path, folder);
        childrenFolders.emplace_back(std::move(folderPtr));
    }
    for (const auto &file : files) {
        std::string tmpPath = FileUtil::SplicePath(path, file);
        childrenFiles.emplace_back(std::make_unique<Protocol::File>(file, tmpPath));
    }
}
} // end of namespace Global
} // end of namespace Module
} // end of namespace Dic