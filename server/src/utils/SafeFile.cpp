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
        message = "Should open file in read mode.";
        Server::ServerLog::Error(message + " path: " + path);
        errMsg = message;
        return res;
    }
    std::string tmpPath = FileUtil::PathPreprocess(path);
    if (!FileUtil::CheckFileValid(tmpPath)) {
        message = "Unable to open file safely, the file path is insecure or not a regular file.";
        Server::ServerLog::Error(message + " path: " + path);
        errMsg = message;
        return res;
    }
    if (!FileUtil::CheckFileSize(path)) {
        message = "Unable to open file safely, the file size does not comply with security regulations.";
        Server::ServerLog::Error(message + " path: " + path);
        errMsg = message;
        return res;
    }
    res.open(tmpPath, std::ios::in | mode);
    return res;
}

std::ifstream OpenWriteFileSafely(const std::string &path, std::ios::openmode mode)
{
    return OpenFileStreamSafely(path, mode | std::ios::out);
}

std::ifstream OpenFileStreamSafely(const std::string &path, std::ios::openmode mode)
{
    std::ifstream res;
    res.setstate(std::ifstream::badbit);
    std::string tmpPath = FileUtil::PathPreprocess(path);
    tmpPath = FileUtil::GetAbsPath(tmpPath);
    std::string filePath = tmpPath;
    if ((mode & std::ios::out) != 0 && !fs::exists(filePath)) {
        filePath = fs::path(tmpPath).parent_path().string();
    }
    // 读取文件和写入文件校验不同
    if (!FileUtil::CheckPathBasic(filePath,
                                  (mode & std::ios::out) == 0 ? fs::perms::owner_read : fs::perms::owner_write)) {
        return res;
    }
    if ((mode & std::ios::in) != 0) {
        if (!FileUtil::CheckFileSize(filePath)) {
            return res;
        }
    }
    res.open(tmpPath, mode);
    return res;
}

}
