/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "FileUtil.h"

namespace Dic {
std::string FileUtil::GetCurrPath()
{
    char currPath[1024];
    std::string strCurrPath;
#ifdef _WIN32
    ::GetModuleFileName(nullptr, currPath, MAX_PATH);
    (_tcsrchr(currPath, '\\'))[1] = 0;
    strCurrPath = StringUtil::GbkToUtf8(currPath);
#else
#ifdef __APPLE__
    uint32_t size = sizeof(currPath);
    if (_NSGetExecutablePath(currPath, &size) == 0) {
        strCurrPath = std::string(dirname(currPath));
    }
#else
    ssize_t len = readlink("/proc/self/exe", currPath, sizeof(currPath) - 1);
    if (len != -1) {
        currPath[len] = '\0';
        strCurrPath = std::string(dirname(currPath));
    }
#endif
#endif
    return strCurrPath;
}

bool FileUtil::CheckFilePathExist(const std::string& filePath)
{
    if (access(filePath.c_str(), R_OK) == -1) {
        Server::ServerLog::Error("Cannot read filePath: ", filePath);
        return false;
    }
    return true;
}

bool FileUtil::CheckFilePath(const std::string& filePath)
{
    if (access(filePath.c_str(), R_OK) == -1) {
        Server::ServerLog::Error("Cannot read" + filePath +": ", filePath);
        return false;
    }
    std::ifstream file(filePath);
    if (!file.good()) {
        Server::ServerLog::Error("Cannot get" + filePath +": ", filePath);
        return false;
    }
    file.close();
    long long size = GetFileSize(filePath.c_str());
    if (size >= MAX_FILE_SIZE_10G) {
        Server::ServerLog::Warn("The size of " + filePath + " is too large in path:", filePath);
    }
    return true;
}

bool FileUtil::CheckFilePathLength(const std::string& filePath)
{
#ifdef _WIN32
    if (filePath.size() >= MAX_PATH) {
        Server::ServerLog::Error("The length of " + filePath + " is too long :", filePath.size());
        return false;
    }
#else
    if (filePath.size() >= PATH_MAX) {
        Server::ServerLog::Error("The length of " + filePath + " is too long :", filePath.size());
        return false;
    }
#endif
    return true;
}

uint32_t FileUtil::GetFilePathLengthLimit()
{
#ifdef _WIN32
    return MAX_PATH;
#else
    return PATH_MAX;
#endif
    return 0;
}

void FileUtil::CalculateDirSize(const std::string &path, long long int &size, int depth)
{
    std::vector<std::string> matchedFiles;
    if (!FileUtil::IsFolder(path)) {
        return;
    }
    std::vector<std::string> folders;
    std::vector<std::string> files;
    if (!FileUtil::FindFolders(path, folders, files)) {
        return;
    }
    for (std::string& folder: folders) {
        std::string spliceFile = SplicePath(path, folder);
        CalculateDirSize(spliceFile, size, depth + 1);
    }
    std::string gbkPath(path);
#ifdef _WIN32
    if (StringUtil::IsUtf8String(path)) {
        gbkPath = StringUtil::Utf8ToGbk(path.c_str());
    }
#endif
    for (std::string& file: files) {
        std::string spliceFile = SplicePath(gbkPath, file);
        if (std::strcmp("ascend_insight_data.db", file.c_str()) != 0 &&
            std::strcmp("cluster.db", file.c_str()) != 0) {
            size += GetFileSize(spliceFile.c_str());
        }
    }
}

std::string FileUtil::GetProfilerFileId(const std::string &filePath)
{
    std::string fileId = FileUtil::GetFileName(filePath);
    std::string grandparentPath = FileUtil::GetParentPath(FileUtil::GetParentPath(filePath));
    if (fileId.empty() || grandparentPath.empty()) {
        return fileId;
    }
    std::vector<std::string> folders;
    std::vector<std::string> files;
    if (!FileUtil::FindFolders(grandparentPath, folders, files)) {
        return fileId;
    }
    if (std::find(folders.begin(), folders.end(), ASCEND_PROFILER_OUTPUT) != folders.end()) {
        // 如果是ASCEND_PROFILER_OUTPUT，则优先匹配profiler_info_x.json，否则取_ascend_pt
        for (const auto& file : files) {
            if (std::regex_match(file, std::regex(PROFILER_INFO_FILE_REG))) {
                return file.substr(PROFILER_INFO_FILE_PREFIX.length(),
                                   file.length() - PROFILER_INFO_FILE_PREFIX.length() - JSON_FILE_SUFFIX.length());
            }
        }
        fileId = FileUtil::GetFileName(grandparentPath);
    } else if (std::find(folders.begin(), folders.end(), MINDSTUDIO_PROFILER_OUTPUT) != folders.end()) {
        // 如果是mindstudio_profiler_output目录，则取Prof_xxx
        fileId = FileUtil::GetFileName(grandparentPath);
    } else if (std::regex_match(FileUtil::GetFileName(grandparentPath), std::regex(DEVICE_DIR_REG))) {
        // 如果是device_x目录，则fileId为x
        fileId = FileUtil::GetFileName(grandparentPath).substr(DEVICE_DIR_PREFIX.length());
    }

    return fileId;
}

std::string FileUtil::GetRootPath(const std::string& filePath)
{
    std::string suffix;
#ifdef _WIN32
    size_t pos = filePath.find_first_of("\\");
    size_t pos2 = filePath.find_first_of("\\/");
    if (pos > pos2) {
        pos = pos2;
    }
    suffix = "\\";
#else
    size_t pos = filePath.find_first_of("\\/");
suffix = "/";
#endif
    if (pos != std::string::npos) {
        return filePath.substr(0, pos) + suffix;
    }
    return "";
}

std::shared_ptr<std::string> FileUtil::GetRelativePath(const std::string& targetFilePath,
                                                       const std::string& sourceFilePath)
{
    // 注意：返回结果可能为空指针，需要进行判断
    std::shared_ptr<std::string> result = nullptr;
    // 对文件目录进行切割，获取两个路径列表
    std::vector<std::string> targetPath = StringUtil::Split(targetFilePath, "[\\\\/]");
    std::vector<std::string> sourcePath = StringUtil::Split(sourceFilePath, "[\\\\/]");
    // 去除列表中空字符串
    targetPath.erase(std::remove_if(targetPath.begin(), targetPath.end(),
        [](const std::string &str) {return str.empty();}), targetPath.end());
    sourcePath.erase(std::remove_if(sourcePath.begin(), sourcePath.end(),
        [](const std::string &str) {return str.empty();}), sourcePath.end());

    // 如果目标路径层级比源路径小，说明目标路径不包含源路径，返回空指针
    if (targetPath.size() < sourcePath.size()) {
        return result;
    }

    // 遍历每层源路径，判断是否和目标路径相等
    int i = 0;
    for (; i < sourcePath.size(); ++i) {
        if (targetPath[i] != sourcePath[i]) {
            break;
        }
    }

    if (i == sourcePath.size()) {
        targetPath.erase(targetPath.begin(), targetPath.begin() + i);
        result = std::make_shared<std::string>(std::accumulate(targetPath.begin(),
            targetPath.end(), std::string(),
            [&](const std::string &a, const std::string &b) -> std::string {
                return a.empty() ? b : a + "/" + b;
            }));
    }
    return result;
}

bool FileUtil::ModifyFilePermissions(const std::string &filePath, const mode_t &mode)
{
    std::string tmpPath(filePath);
#ifdef _WIN32
    if (StringUtil::IsUtf8String(filePath)) {
        tmpPath = StringUtil::Utf8ToGbk(filePath.c_str());
    }
#endif
    return chmod(tmpPath.c_str(), mode);
}
}