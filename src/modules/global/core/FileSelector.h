/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_FILE_SELECTOR_H
#define PROFILER_SERVER_FILE_SELECTOR_H

#include <string>
#include <vector>
#include <memory>
#include "GlobalProtocolResponse.h"

namespace Dic {
namespace Module {
namespace Global {
class FileSelector {
public:
    FileSelector() = default;
    ~FileSelector() = default;

    static void GetFoldersAndFiles(const std::string &path,
                                   std::vector<std::unique_ptr<Protocol::Folder>> &childrenFolders,
                                   std::vector<std::string> &childrenFiles, int depth = 0);

private:
    static const int maxDepth = 2;
};
} // end of namespace Global
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_FILE_SELECTOR_H
