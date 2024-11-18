/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "SafeFile.h"

namespace Dic {
std::ifstream OpenReadFileSafely(const std::string &path, std::ios::openmode mode)
{
    std::ifstream res;
    res.setstate(std::ifstream::badbit);
    if (mode & std::ios::out) {
        Server::ServerLog::Error("Should open file in read mode, path: ", path);
        return res;
    }
    std::string tmpPath = FileUtil::PathPreprocess(path);
    if (!FileUtil::CheckFileValid(tmpPath)) {
        Server::ServerLog::Error("Open read file safely failed, path: ", path);
        return res;
    }
    if (!FileUtil::CheckFileSize(path)) {
        Server::ServerLog::Error("Open read file safely failed, "
                                 "The file size does not comply with security regulations. path: ", path);
        return res;
    }
    res.open(tmpPath, std::ios::in | mode);
    return res;
}
}
