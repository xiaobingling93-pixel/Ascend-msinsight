/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <unordered_map>
#include <cstring>
#include "FileUtil.h"

namespace Dic {
namespace {
// 非Windows平台不允许反斜杠
#ifdef _WIN32
const std::unordered_map<std::string, std::string> INVALID_CHAR = {
    {"\n", "\\n"}, {"\f", "\\f"}, {"\r", "\\r"}, {"\b", "\\b"},
    {"\t", "\\t"}, {"\v", "\\v"}, {"\x7F", "\\x7F"}, {"\u007F", "\\u007F"},
    {"\"", "\\\""}, {"\'", "\\\'"}, {"%", "\\%"}, {">", "\\>"}, {"<", "\\<"},
    {"|", "\\|"}, {"&", "\\&"}, {"$", "\\$"}, {";", "\\;"}, {"`", "\\`"}
};
#else
const std::unordered_map<std::string, std::string> INVALID_CHAR = {
    {"\n", "\\n"}, {"\f", "\\f"}, {"\r", "\\r"}, {"\b", "\\b"},
    {"\t", "\\t"}, {"\v", "\\v"}, {"\x7F", "\\x7F"}, {"\u007F", "\\u007F"},
    {"\"", "\\\""}, {"\'", "\\\'"}, {"%", "\\%"}, {">", "\\>"}, {"<", "\\<"},
    {"|", "\\|"}, {"&", "\\&"}, {"$", "\\$"}, {";", "\\;"}, {"`", "\\`"},
    {"\\", "\\\\"}
};
#endif
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
    if (!path.empty() && path[0] == '/') {
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

bool FileUtil::IsRegularFile(const std::string &filePath)
{
#ifdef _WIN32
    DWORD fileAttributes = GetFileAttributesA(filePath.c_str());
    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        return false;
    }
    return S_ISREG(fileStat.st_mode);
#endif
}

bool FileUtil::CheckFileValid(const std::string &filePath)
{
    if (!CheckDirValid(filePath)) {
        Server::ServerLog::Error("The file path is insecure.");
        return false;
    }
    if (!IsRegularFile(filePath)) {
        Server::ServerLog::Error("The file is not a regular file.");
        return false;
    }
    return true;
}

bool FileUtil::CheckDirValid(const std::string &path)
{
    if (path.empty()) {
        Server::ServerLog::Error("The path is empty. ");
        return false;
    }
    std::string dir = GetAbsPath(path);
    if (dir.empty()) {
        Server::ServerLog::Error("Failed to retrieve the absolute path.");
        return false;
    }
    if (!CheckFilePathLength(dir)) {
        return false;
    }
    if (CheckPathInvalidChar(dir)) {
            return false;
    }
    if (!CheckDirAccess(dir, 0)) {
        Server::ServerLog::Error("The directory path not exists. path: %.", dir);
        return false;
    }
    if (IsSoftLink(dir)) {
        Server::ServerLog::Error("The path is soft link. path: %.", dir);
        return false;
    }
    if (!CheckDirAccess(dir, R_OK)) {
        Server::ServerLog::Error("The path has no read access. path: %.", dir);
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
        Server::ServerLog::Error("Check file path exist cannot read file path");
        return false;
    }
    return true;
}

bool FileUtil::CheckFilePath(const std::string& filePath)
{
    std::string tempPath = FileUtil::PathPreprocess(filePath);
    // 文件基础校验：校验文件最小权限、软连接、长度、特殊字符
    if (!CheckDirValid(tempPath)) {
        Server::ServerLog::Error("Invalid file path. There may be issues with file permissions, soft link, length,"
                                 " or special characters");
        return false;
    }
    std::ifstream file(tempPath);
    if (!file.good()) {
        Server::ServerLog::Error("Fail to open file path");
        return false;
    }
    file.close();
    long long size = GetFileSize(tempPath.c_str());
    if (size >= MAX_FILE_SIZE_10G) {
        Server::ServerLog::Warn("The size of file is too large");
    }
    return true;
}

bool FileUtil::CheckFilePathLength(const std::string& filePath)
{
#ifdef _WIN32
    if (filePath.size() >= MAX_PATH) {
        Server::ServerLog::Error("The path length of % exceeds the maximum allowed length of % characters."
                                 "The file size is % MB", filePath, MAX_PATH, filePath.size());
        return false;
    }
#else
    if (filePath.size() >= PATH_MAX) {
        Server::ServerLog::Error("The path length of % exceeds the maximum allowed length of % characters."
                                 "The file size is % MB", filePath, PATH_MAX, filePath.size());
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
    std::string error;
    if (!IsWithinRecursionLimit(files, depth, error)) {
        Server::ServerLog::Error("Directory size calculation aborted. Reason: " + error);
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
    // db优先:如果text和db数据共存，则优先判断为db
    bool hasJson = false;
    for (const auto &file: files) {
        if (std::regex_match(file, dbRegex)) {
            return dbFound;
        }
        hasJson = hasJson || std::regex_match(file, jsonRegex);
    }

    for (const auto &folder: folders) {
        std::string tmpPath = FileUtil::SplicePath(path, folder);
        int result = FindDbOrJsonType(tmpPath, depth + 1, jsonRegex, dbRegex);
        if (result != 0) {
            return result;
        }
    }
    return hasJson ? jsonFound : 0;
}

// 递归查找函数
std::vector<std::string> FileUtil::FindFirstByRegex(const std::string &path, int depth, const std::regex &fileRegex)
{
    std::vector<std::string> matchedFiles;
    int maxDepth = 5;
    if (depth > maxDepth) {
        return matchedFiles;
    }
    std::vector<std::string> folders;
    std::vector<std::string> files;
    if (!FileUtil::FindFolders(path, folders, files)) {
        return matchedFiles;
    }
    sort(files.begin(), files.end(), std::greater<std::string>());
    for (const auto &file: files) {
        if (std::regex_match(file, fileRegex)) {
            auto aimFile = FileUtil::SplicePath(path, file);
            matchedFiles.push_back(aimFile);
            return matchedFiles;
        }
    }
    for (const auto &folder: folders) {
        std::string tmpPath = FileUtil::SplicePath(path, folder);
        auto foundFiles = FindFirstByRegex(tmpPath, depth + 1, fileRegex);
        if (!foundFiles.empty()) {
            matchedFiles.insert(matchedFiles.end(), foundFiles.begin(), foundFiles.end());
            return matchedFiles;
        }
    }
    return matchedFiles;
}

bool FileUtil::CheckFileSize(const std::string &filePath)
{
    constexpr size_t fileMinSize = 0;
    constexpr size_t fileMaxSize = 20ULL * 1024 * 1024 * 1024;
#ifdef _WIN32
    std::string tmpFilePath = FileUtil::PathPreprocess(filePath);
    WIN32_FILE_ATTRIBUTE_DATA fileData;
    if (GetFileAttributesEx(tmpFilePath.c_str(), GetFileExInfoStandard, &fileData)) {
        // 获取文件大小
        uintmax_t fileSize = (static_cast<uintmax_t>(fileData.nFileSizeHigh) << 32) | fileData.nFileSizeLow;
        if (fileSize <= fileMinSize) {
            Server::ServerLog::Error("This file % is an empty file.", tmpFilePath);
            return false;
        }
        if (fileSize > fileMaxSize) {
            Server::ServerLog::Error("The size limit for the file located at tmpFilePath has been exceeded.",
                                     tmpFilePath);
            return false;
        }
        return true;
    }
#else
    // 获取文件大小
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) == 0) {
        if (fileStat.st_size < 0) {
            Server::ServerLog::Error("Size of this file is smaller than 0.");
            return false;
        }
        size_t fileSize = static_cast<size_t>(fileStat.st_size);
        if (fileSize <= fileMinSize) {
            Server::ServerLog::Error("This file % is an empty file.", filePath);
            return false;
        }
        if (fileSize > fileMaxSize) {
            Server::ServerLog::Error("The size limit for the file located at % has been exceeded.",
                                     filePath);
            return false;
        }
        return true;
    }
#endif
    return false;
}

std::string FileUtil::GetDbPath(const std::string &filePath, const std::string &fileId)
{
    std::string dbPath = GetDbPath(filePath);
    std::string tmpFileId = GetProfilerFileId(filePath);
    if (tmpFileId.length() < fileId.length() && fileId.find(tmpFileId) == 0) {
        // 修改db文件名为mindstudio_insight_data_xxx.db
        dbPath = dbPath.substr(0, dbPath.length() - DB_FILE_SUFFIX.length()) +
                 fileId.substr(tmpFileId.length()) + DB_FILE_SUFFIX;
    }
    return dbPath;
}

long long FileUtil::GetFileSize(const char *fileName)
{
    if (!fileName) {
        return 0;
    }

    if (strcmp(fileName, "") == 0) {
        return 0;
    }
#ifdef _WIN32
    std::string tmpFilePath = FileUtil::PathPreprocess(fileName);
    WIN32_FILE_ATTRIBUTE_DATA fileData;
    if (GetFileAttributesEx(tmpFilePath.c_str(), GetFileExInfoStandard, &fileData)) {
        // 获取文件大小
        uintmax_t fileSize = (static_cast<uintmax_t>(fileData.nFileSizeHigh) << 32) | fileData.nFileSizeLow;
        return fileSize;
    }
    return 0;
#else
    struct stat st;
        long long size = 0;
        if (stat(fileName, &st) == 0) {
            size = st.st_size;
        }
        return size;
#endif
}

std::string FileUtil::GetDbPath(const std::string &filePath)
{
    std::string grandparentPath = FileUtil::GetParentPath(FileUtil::GetParentPath(filePath));
    if (grandparentPath.empty()) {
        return FileUtil::SplicePath(FileUtil::GetParentPath(filePath), DATABASE_FILE_NAME);
    }
    std::vector<std::string> folders;
    std::vector<std::string> files;
    if (!FileUtil::FindFolders(grandparentPath, folders, files)) {
        return FileUtil::SplicePath(FileUtil::GetParentPath(filePath), DATABASE_FILE_NAME);
    }
    if (std::find(folders.begin(), folders.end(), ASCEND_PROFILER_OUTPUT) != folders.end()) {
        // 如果是ASCEND_PROFILER_OUTPUT，则放在ASCEND_PROFILER_OUTPUT下
        auto directory = FileUtil::SplicePath(grandparentPath, ASCEND_PROFILER_OUTPUT);
        return FileUtil::SplicePath(directory, DATABASE_FILE_NAME);
    } else if (std::find(folders.begin(), folders.end(), MINDSTUDIO_PROFILER_OUTPUT) != folders.end()) {
        // 如果是mindstudio_profiler_output目录，则放在mindstudio_profiler_output
        auto directory = FileUtil::SplicePath(grandparentPath, MINDSTUDIO_PROFILER_OUTPUT);
        return FileUtil::SplicePath(directory, DATABASE_FILE_NAME);
    } else if (std::regex_match(FileUtil::GetFileName(grandparentPath), std::regex(DEVICE_DIR_REG))) {
        // 如果是device_x目录，则放在device_x的上层目录
        return FileUtil::SplicePath(grandparentPath, DATABASE_FILE_NAME);
    }

    return FileUtil::SplicePath(FileUtil::GetParentPath(filePath), DATABASE_FILE_NAME);
}
bool FileUtil::CheckPathInvalidChar(const std::string &filePath)
{
    for (auto &item : INVALID_CHAR) {
        if (filePath.find(item.first) != std::string::npos) {
            Server::ServerLog::Error("The path: % contains invalid character: %.", filePath, item.second);
            return true;
        }
    }
    return false;
}

static inline std::vector<std::function<std::tuple<bool, const char*>(const std::string&)>> CheckPathBasicConditions = {
    [](const std::string &path) {
        return std::make_tuple(path.empty(), "Failed to retrieve the absolute path");
    }, // 是否为空
    [](const std::string &path) {
        return std::make_tuple(!FileUtil::CheckFilePathLength(path), "The file path exceed limit");
    }, // 是否过长
    [](const std::string &path) {
        return std::make_tuple(FileUtil::CheckPathInvalidChar(path), "The file path contains invalid char");
    }, // 是否含有非法字符
    [](const std::string &path) {
        return std::make_tuple(!fs::exists(path), "The file path not exist");
    }, // 是否存在
    [](const std::string &path) {
        return std::make_tuple(FileUtil::IsSoftLink(path), "The file path is soft link");
    }, // 是否软链
    [](const std::string &path) {
        return std::make_tuple(!FileUtil::CheckPathOwner(path), "The file path owner not current user");
    }, // 是否非属主
    [](const std::string &path) {
        const std::string parentDir = fs::path(path).parent_path().string();
        return std::make_tuple(!parentDir.empty() && !FileUtil::CheckPathOwner(parentDir),
            "The parent file dir owner not right");
    }, // 是否非父级目录属主
    [](const std::string &path) {
        return std::make_tuple(!fs::is_directory(path) && !FileUtil::IsRegularFile(path),
            "The file is not regular file");
    }, // 是否非常规文件/目录
};

Status FileUtil::CheckPathBasic(const std::string &filePath, fs::perms permission)
{
    Status s(true);
    if (filePath.empty()) {
        Server::ServerLog::Error("The path is empty. path:", filePath);
        s.SetErr("The path is empty");
        return s;
    }
    const std::string absPath = GetAbsPath(filePath);
    try {
        for (size_t i = 0;i < CheckPathBasicConditions.size(); ++i) {
            if (auto [ failed, onFailedMsg ] = CheckPathBasicConditions[i](absPath); failed) {
                Server::ServerLog::Error("%s. Path:%s", onFailedMsg, absPath);
                s.SetErr(onFailedMsg);
                return s;
            }
        }
        if (permission != fs::perms::none && !CheckPathPermission(absPath, permission)) {
            Server::ServerLog::Error("The file permission is not correct, path: ", filePath);
            s.SetErr("The file permission is not correct");
            return s;
        }
    } catch (const fs::filesystem_error& e) {  // filesystem would throw exception sometimes
        Server::ServerLog::Error("Basic file check failed by system exception (%s), path:(%s)", e.what(), filePath);
        s.SetErr("Basic file check failed by system exception");
        return s;
    }
    return s;
}

bool FileUtil::CheckPathOwner(const std::string &filePath)
{
#ifdef _WIN32
    PSECURITY_DESCRIPTOR pSD = nullptr;
    PSID pOwner = nullptr;
    if (GetNamedSecurityInfoA(filePath.c_str(), SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, &pOwner, nullptr, nullptr,
                              nullptr, &pSD) != ERROR_SUCCESS) {
        Server::ServerLog::Warn("Get file owner failed");
        return false;
    }
    HANDLE tokenHandle;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle)) {
        Server::ServerLog::Warn("Get current owner failed");
        return false;
    }
    DWORD tokenInfoLength = 0;
    GetTokenInformation(tokenHandle, TokenUser, nullptr, 0, &tokenInfoLength);
    TOKEN_USER *tokenUser = (TOKEN_USER *) malloc(tokenInfoLength);
    if (!GetTokenInformation(tokenHandle, TokenUser, tokenUser, tokenInfoLength, &tokenInfoLength)) {
        free(tokenUser);
        CloseHandle(tokenHandle);
        if (pSD) {
            LocalFree(pSD);
        }
        return false;
    }
    bool isOwner = EqualSid(pOwner, tokenUser->User.Sid);
    free(tokenUser);
    CloseHandle(tokenHandle);
    if (pSD) {
        LocalFree(pSD);
    }
    return isOwner;
#else
    struct stat fileStat{};
    if (stat(filePath.c_str(), &fileStat) != 0) {
        Server::ServerLog::Warn("Get file info failed when check owner");
        return false;
    }
    uid_t fileOwner = fileStat.st_uid;
    uid_t currentUser = geteuid();
    return fileOwner == currentUser;
#endif
}

bool FileUtil::CheckPathPermission(const std::string &filePath, fs::perms permission)
{
    if (filePath.empty()) {
        return false;
    }
    if (!fs::exists(filePath)) {
        return false;
    }
    try {
        auto perms = fs::status(filePath).permissions();
        if ((perms & permission) != fs::perms::none) {
            return true;
        } else {
            Server::ServerLog::Warn("File % doesn't contain the permission %", filePath, static_cast<int>(permission));
            return false;
        }
    } catch (const fs::filesystem_error &e) {
        Server::ServerLog::Error("Get file permission failed, error:", e.what());
        return false;
    }
}
} // Dic
