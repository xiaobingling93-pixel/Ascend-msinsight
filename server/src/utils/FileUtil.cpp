/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <unordered_map>
#include "FileUtil.h"

namespace Dic {
namespace {
const std::unordered_map<std::string, std::string> INVALID_CHAR = {
    {"\n", "\\n"}, {"\f", "\\f"}, {"\r", "\\r"}, {"\b", "\\b"},
    {"\t", "\\t"}, {"\v", "\\v"}, {"\x7F", "\\x7F"}
};
#ifdef _WIN32
const std::string_view MSVP_SLASH = "\\";
#else
const std::string_view MSVP_SLASH = "/";
#endif
}
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

bool FileUtil::IsAbsolutePath(const std::string &path)
{
#ifdef _WIN32
    if (path.length() < 2) { // 2表示两个字符 d: 小于两个就一定不是绝对路径
        return false;
    }
    // 判断是否以盘符开头
    if (isalpha(path[0]) && path[1] == ':') {
        return true;
    }
#else
    if (path[0] == '/') {
        return true;
    }
#endif
    return false;
}

// 这里需要修改，windows和linux下不一样，另外日志也不太能帮助定义，
std::string FileUtil::GetAbsPath(const std::string &path)
{
    if (path.empty()) {
        return "";
    }
    if (IsAbsolutePath(path)) {
        return std::string(path);
    }
    std::string curPath = GetCurrPath();
    if (curPath.empty()) {
        Server::ServerLog::Error("Failed to retrieve the current path.");
        return "";
    }
    return curPath + std::string(MSVP_SLASH) + path;
}
 
bool FileUtil::IsSoftLink(const std::string &path)
{
#ifdef _WIN32
    std::wstring widePath(path.begin(), path.end());
    DWORD attributes = GetFileAttributesW(widePath.c_str());
    return (attributes != INVALID_FILE_ATTRIBUTES) &&
           (attributes & FILE_ATTRIBUTE_REPARSE_POINT);
#else
    struct stat fileStat;
    if (lstat(path.c_str(), &fileStat) != 0) {
        Server::ServerLog::Error("The file lstat failed.");
        return false;
    }
    return S_ISLNK(fileStat.st_mode);
#endif
}
 
bool FileUtil::CheckDirValid(const std::string &path)
{
    if (path.empty()) {
        Server::ServerLog::Error("The path is empty. ");
        return false;
    }
    std::string dir = GetAbsPath(path);
    if (dir.empty()) {
        Server::ServerLog::Error("Failed to retrieve the absolute path. path: ", path);
        return false;
    }
 
    if (!CheckFilePathLength(dir)) {
        return false;
    }
 
    for (auto &item: INVALID_CHAR) {
        if (dir.find(item.first) != std::string::npos) {
            Server::ServerLog::Error("The path: ", dir, " contains invalid character: ", item.second);
            return false;
        }
    }
 
    if (!CheckDirAccess(dir, 0)) {
        Server::ServerLog::Error("The directory path not exists. path: ", dir);
        return false;
    }
 
    if (IsSoftLink(dir)) {
        Server::ServerLog::Error("The path is soft link. path: ", dir);
        return false;
    }
 
    if (!CheckDirAccess(dir, R_OK)) {
        Server::ServerLog::Error("The path have no read access. path: ", dir);
        return false;
    }
    return true;
}

bool FileUtil::CheckFilePathExist(const std::string& filePath)
{
    std::string tmpPath(filePath);
#ifdef _WIN32
    if (StringUtil::IsUtf8String(filePath)) {
        tmpPath = StringUtil::Utf8ToGbk(filePath.c_str());
    }
#endif
    if (access(tmpPath.c_str(), R_OK) == -1) {
        Server::ServerLog::Error("Cannot read filePath: ", tmpPath);
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
        if (std::strcmp(DATABASE_FILE_NAME.c_str(), file.c_str()) != 0 &&
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
    size_t i = 0;
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

bool FileUtil::ConvertToRealPath(std::string &errorMsg, std::vector<std::string> &path)
{
    for (auto it = path.begin(); it != path.end(); ++it) {
        if (!FileUtil::CheckDirValid(*it)) {
            errorMsg = *it + "is invalid path";
            return false;
        }
        std::string realPath = GetRealPath(*it);
        if (realPath.empty()) {
            errorMsg = "The conversion of the " + *it +
                        "test path to an absolute path has failed.";
            return false;
        }
        *it = realPath;
    }
    return true;
}

bool FileUtil::FindIfDbTypeByRegex(const std::string &path, const std::regex &jsonRegex,
                                   const std::regex &dbRegex)
{
    int dbFound = 2;
    return FileUtil::FindDbOrJsonType(path, 0, jsonRegex, dbRegex) == dbFound;
}

int FileUtil::FindDbOrJsonType(const std::string &path, int depth,
                               const std::regex &jsonRegex, const std::regex &dbRegex)
{
    int jsonFound = 1;
    int dbFound = 2;
    int maxDepth = 5;
    if (depth > maxDepth) {
        return jsonFound;
    }
    std::vector<std::string> folders;
    std::vector<std::string> files;
    if (!FileUtil::FindFolders(path, folders, files)) {
        return jsonFound;
    }
    sort(files.begin(), files.end(), std::greater<std::string>());
    for (const auto &file: files) {
        if (std::regex_match(file, dbRegex)) {
            return dbFound;
        } else if (std::regex_match(file, jsonRegex)) {
            return jsonFound;
        }
    }
    for (const auto &folder: folders) {
        std::string tmpPath = FileUtil::SplicePath(path, folder);
        int result = FindDbOrJsonType(tmpPath, depth + 1, jsonRegex, dbRegex);
        if (result != 0) {
            return result;
        }
    }
    return 0;
}


// 递归查找函数
std::vector<std::string> FileUtil::FindFirstByRegex(const fs::path &path, int depth, const std::regex &fileRegex)
{
    std::vector<std::string> matchedFiles;
    int maxDepth = 5;
    if (depth > maxDepth) {
        return matchedFiles;
    }
    for (const auto &entry: fs::directory_iterator(path)) {
        if (fs::is_directory(entry.path())) {
            auto foundFiles = FindFirstByRegex(entry, depth + 1, fileRegex);
            if (!foundFiles.empty()) {
                matchedFiles.insert(matchedFiles.end(), foundFiles.begin(), foundFiles.end());
                return matchedFiles;
            }
        } else if (std::regex_match(entry.path().filename().string(), fileRegex)) {
            matchedFiles.push_back(fs::absolute(entry.path()).string());
            return matchedFiles;
        }
    }
    return matchedFiles;
}
} // Dic
