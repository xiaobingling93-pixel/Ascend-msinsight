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
                                      std::vector<std::unique_ptr<Protocol::File>> &childrenFiles)
{
    std::vector<std::string> folders;
    std::vector<std::string> files;
    if (path.empty()) {
        folders = FileUtil::GetDiskInfo();
    } else {
        FileUtil::FindFolders(path, folders, files);
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