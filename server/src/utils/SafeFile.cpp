/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "SafeFile.h"

namespace Dic {
std::ifstream OpenReadFileSafely(const std::string &path, std::ios::openmode mode, std::string &errMsg)
{
    std::ifstream res;
    res.setstate(std::ifstream::badbit);
    std::string message;
    if (mode & std::ios::out) {
        message = "Should open file in read mode, path: " + path;
        Server::ServerLog::Error(message);
        errMsg = message;
        return res;
    }
    std::string tmpPath = FileUtil::PathPreprocess(path);
    // check if file can be written by others
    if (FileUtil::CheckPathPermission(tmpPath, fs::perms::others_write)) {
        message = "Unable to open file safely, the file can be written by others, path: " + path;
        Server::ServerLog::Error(message);
        errMsg = message;
        return res;
    }
    if (!FileUtil::CheckFileValid(tmpPath)) {
        message = "Unable to open file safely, the file path is insecure or not a regular file, path: " + path;
        Server::ServerLog::Error(message);
        errMsg = message;
        return res;
    }
    if (!FileUtil::CheckFileSize(path)) {
        message = "Unable to open file safely, the file size does not comply with security regulations. path: " + path;
        Server::ServerLog::Error(message);
        errMsg = message;
        return res;
    }
    res.open(tmpPath, std::ios::in | mode);
    return res;
}
}
