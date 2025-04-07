/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "SafeFile.h"

namespace Dic {
std::ifstream OpenReadFileSafely(const std::string &path, std::ios::openmode mode)
{
    std::ifstream res;
    res.setstate(std::ifstream::badbit);
    std::string message;
    if (mode & std::ios::out) {
        message = "Should open file in read mode.";
        Server::ServerLog::Error(message + " path: " + path);
        return res;
    }
    std::string tmpPath = FileUtil::PathPreprocess(path);
    if (!FileUtil::CheckFileValid(tmpPath)) {
        message = "Unable to open file safely, the file path is insecure or not a regular file.";
        Server::ServerLog::Error(message + " path: " + path);
        return res;
    }
    if (!FileUtil::CheckFileSize(path)) {
        message = "Unable to open file safely, the file size does not comply with security regulations.";
        Server::ServerLog::Error(message + " path: " + path);
        return res;
    }
    res.open(tmpPath, std::ios::in | mode);
    return res;
}
}
