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
                                      std::vector<std::string> &childrenFiles,
                                      int depth)
{
    if (depth >= maxDepth) {
        return;
    }
    std::vector<std::string> folders;
    if (path.empty()) {
        folders = FileUtil::GetDiskInfo();
    } else {
        auto files = FileUtil::FindFolders(path);
        for (const auto &file : files) {
            std::string tmpPath = FileUtil::SplicePath(path, file);
            if (FileUtil::IsHiddenPath(tmpPath)) {
                continue;
            }
            if (FileUtil::IsFolder(tmpPath)) {
                folders.emplace_back(file);
            } else {
                childrenFiles.emplace_back(file);
            }
        }
    }
    depth = folders.size() == 1 ? depth : depth + 1; // 只有一层目录的，继续向下展开
    for (const auto &folder : folders) {
        auto folderPtr = std::make_unique<Folder>();
        folderPtr->name = folder;
        std::string tmpPath = FileUtil::SplicePath(path, folder);
        GetFoldersAndFiles(tmpPath, folderPtr->childrenFolders, folderPtr->childrenFiles, depth);
        childrenFolders.emplace_back(std::move(folderPtr));
    }
}
} // end of namespace Global
} // end of namespace Module
} // end of namespace Dic