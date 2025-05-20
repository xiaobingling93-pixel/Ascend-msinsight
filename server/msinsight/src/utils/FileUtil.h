/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_FILEUTIL_H
#define DATA_INSIGHT_CORE_FILEUTIL_H

#include <string>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include <sys/stat.h>
#include <fstream>
#include <libgen.h>
#include <numeric>
#include "regex"
#include "RegexUtil.h"
#include "ServerLog.h"
#include "FileDef.h"
#include "StringUtil.h"
#include "Status.h"

#if defined(_WIN32)
#include <filesystem>
#include <windows.h>
#include <shlwapi.h>
#include <io.h>
#include <tchar.h>
#include <aclapi.h>
#include <sddl.h>
namespace fs = std::filesystem;
#else
#include <sys/stat.h>
#include <unistd.h>
#include <climits>
#include <sys/wait.h>
#include "dlfcn.h"
#endif
#ifdef __linux__
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#ifdef __APPLE__
#include <filesystem>
#include <mach-o/dyld.h>
namespace fs = std::filesystem;
#endif
#ifdef _WIN32
#define FILE_SEPARATOR '\\'
#else
#define FILE_SEPARATOR '/'
#endif

namespace Dic {
const std::string DB_REG =
        R"(((msprof_[0-9]{1,16}|((ascend_pytorch_profiler)(_[0-9]{1,16}){0,1})|)"
        R"(((ascend_mindspore_profiler)(_[0-9]{1,16}){0,1})|cluster_analysis|leaks_dump_\d+)\.db$))";
class FileUtil {
public:
    static inline bool CheckDirAccess(const std::string &path, const int &mode = 0)
    {
#ifdef _WIN32
        std::string tmpPath(path);
        if (_access(tmpPath.c_str(), mode) == -1) {
            return false;
        }
#else
        if (access(path.c_str(), mode) == -1) {
            return false;
        }
#endif
        return true;
    }

    static inline std::string SplicePath(const std::string &path)
    {
        return path;
    }

    template<typename... Args>
    static inline std::string SplicePath(const std::string &first, Args... args)
    {
        std::string res(first);
#ifdef _WIN32
        if (!first.empty() && first[first.size() - 1] != '/' && first[first.size() - 1] != '\\') {
            return first + "\\" + SplicePath(args...);
        }
#else
        if (!first.empty() && first[first.size() - 1] != '/') {
            return first + "/" + SplicePath(args...);
        }
#endif
        return res + SplicePath(args...);
    }

    static inline std::string GetRealPath(const std::string &path)
    {
#ifdef _WIN32
        wchar_t wResolvedPath[MAX_PATH] = {0}; // windows中文路径需要使用宽字符处理
        if (_wfullpath(wResolvedPath, StringUtil::String2WString(path).data(),
                       MAX_PATH) == nullptr) {
            return "";
        }
        return StringUtil::WString2String(wResolvedPath).data();
#else
        char resolvedPath[PATH_MAX] = {0};
        if (realpath(path.c_str(), resolvedPath) == nullptr) {
            return "";
        }
        return std::string(resolvedPath);
#endif
    }

    static inline std::string GetFileName(const std::string &path)
    {
        if (path.empty()) {
            return "";
        }
        auto pos = path.find_last_of('/');
#ifdef _WIN32
        if (pos == std::string::npos) {
            pos = path.find_last_of('\\');
        } else if (path.find_last_of('\\') != std::string::npos) {
            pos = std::max(pos, path.find_last_of('\\'));
        }
#endif
        if (pos == std::string::npos) {
            return path;
        }
        return path.substr(pos + 1);
    }

#ifdef _WIN32
    static inline bool FindFolders(const std::string &path,
        std::vector<std::string> &folders, std::vector<std::string> &files)
    {
        // long type will crash when use wingw11 compile in windows11
        long long hFile = 0;
        const uint64_t fileCountLimit = 100000;
        struct _finddata_t fileInfo {};
        std::string tmpPath = PathPreprocess(path);
        std::string currentPath = PathPreprocess(path);
        if ((hFile = _findfirst(tmpPath.append("\\*").c_str(), &fileInfo)) == -1) {
            return false;
        }
        do {
            if ((fileInfo.attrib & (_A_HIDDEN | _A_SYSTEM)) != 0) {
                continue;
            }
            if (std::string(fileInfo.name) == ".." || std::string(fileInfo.name) == ".") {
                continue;
            }
            if (!CheckDirValid(SplicePath(currentPath, fileInfo.name))) {
                continue;
            }
            if ((fileInfo.attrib & _A_SUBDIR) != 0) {
                folders.emplace_back(fileInfo.name);
            } else {
                files.emplace_back(fileInfo.name);
            }
            if (folders.size() + files.size() > fileCountLimit) {
                Server::ServerLog::Warn("There are too many sub files in the folder");
                folders.clear();
                files.clear();
                break;
            }
        } while (_findnext(hFile, &fileInfo) == 0);
        _findclose(hFile);
        return true;
    }
#else
    static inline bool FindFolders(const std::string &path,
                                   std::vector<std::string> &folders,
                                   std::vector<std::string> &files)
    {
        if (path.empty()) {
            return false;
        }
        DIR *pDir = nullptr;
        struct dirent *pDirent = nullptr;
        struct stat pathStat;
        std::string tmpPath(path);
        pDir = opendir(tmpPath.c_str());
        if (pDir == nullptr) {
            return false;
        }
        const uint64_t fileCountLimit = 100000;
        while ((pDirent = readdir(pDir)) != nullptr) {
            if (std::string(pDirent->d_name) == ".." || std::string(pDirent->d_name) == ".") {
                continue;
            }
            std::string fullPath = tmpPath + "/" + pDirent->d_name;
            if (stat(fullPath.c_str(), &pathStat) != 0) {
                continue;
            }
            if (!CheckDirValid(fullPath)) {
                continue;
            }
            if (S_ISDIR(pathStat.st_mode)) {
                folders.emplace_back(pDirent->d_name);
            } else if (S_ISREG(pathStat.st_mode)) {
                files.emplace_back(pDirent->d_name);
            } else {
                Server::ServerLog::Info("Other type : [", pathStat.st_mode, "] : [", pDirent->d_name);
            }
            if (folders.size() + files.size() > fileCountLimit) {
                Server::ServerLog::Warn("There are too many sub files in the folder");
                folders.clear();
                files.clear();
                break;
            }
        }
        closedir(pDir);
        return true;
    }
#endif

    static inline bool IsFolder(const std::string &path)
    {
#ifdef _WIN32
        std::string tmpPath(path);
        return PathIsDirectory(tmpPath.c_str());
#else
        struct stat st;
        if (stat(path.c_str(), &st) == -1) {
            return false;
        }
        return S_ISDIR(st.st_mode);
#endif
    }

    static inline bool RemoveFile(const std::string &path)
    {
        std::string tmpPath(path);
        return std::remove(tmpPath.c_str()) == 0;
    }

    static inline bool RemoveFileExDb(const std::string &path)
    {
        std::string fineName = GetFileName(path);
        bool isDbFile = std::regex_match(path, std::regex(DB_REG));
        if (isDbFile) {
            return true;
        }
        return RemoveFile(path);
    }
    /**
     * 根据文件路径复制文件。
     * @param sourceFilePath 源文件路径
     * @param targetFilePath 目标文件路径（需保证目标文件父目录存在，无需目标文件本身存在。）
     * @return
     */
    static inline bool CopyFileByPath(const std::string &sourceFilePath, const std::string &targetFilePath)
    {
        // 检查 源文件路径 合法性，包括文件最小权限、软连接、长度、特殊字符
        if (!CheckFilePath(sourceFilePath)) {
            Server::ServerLog::Error("Source file path is invalid when copy file.");
            return false;
        }
        // 目标文件目录
        std::string targetDirPath = GetParentPath(targetFilePath);
        // 检查 目标文件目录 合法性、是否可写
        if (!CheckDirValid(targetDirPath) || !CheckDirAccess(targetDirPath, W_OK)) {
            Server::ServerLog::Error("Target directory is invalid or not writable when copy file.");
            return false;
        }
#ifdef _WIN32
        std::string tmpSourcePath(sourceFilePath);
        std::string tmpTargetPath(targetFilePath);
        return CopyFile(tmpSourcePath.c_str(), tmpTargetPath.c_str(), false);
#else
        pid_t pid = fork();
        if (pid == -1) {
            // fork进程失败
            return false;
        } else if (pid == 0) {
            // 子进程执行cp命令
            execl("/bin/cp", "cp", sourceFilePath.c_str(), targetFilePath.c_str(), NULL);
            // 如果子进程执行成功，则不会继续往下执行
            return false;
        } else {
            // 等待子进程结束，判断子进程执行情况
            int status;
            waitpid(pid, &status, 0);
            return WIFEXITED(status) && WEXITSTATUS(status) == 0;
        }
#endif
    }

    static inline std::vector<std::string> GetDiskInfo()
    {
        std::vector<std::string> disk;
#ifdef _WIN32
        DWORD dwSize = MAX_PATH;
        char szLogicalDrives[MAX_PATH] = {0};
        DWORD dwResult = GetLogicalDriveStrings(dwSize - 1, szLogicalDrives);
        if (dwResult > 0 && dwResult <= MAX_PATH) {
            auto *szSingleDrive = szLogicalDrives;
            while (*szSingleDrive) {
                disk.emplace_back(szSingleDrive);
                szSingleDrive += strlen(szSingleDrive) + 1;
            }
        }
#else
        disk.emplace_back("/");
#endif
        return disk;
    }

    static inline std::map<std::string, std::vector<std::string>>
    SplitToRankList(std::vector<std::pair<std::string, std::string>> fileList)
    {
        std::map<std::string, std::vector<std::string>> rankListMap;
        for (const auto &item: fileList) {
            rankListMap[item.second].push_back(item.first);
        }
        return rankListMap;
    }

    static inline std::string GetRankIdFromFile(std::string timeLineFile)
    {
        std::string parent = GetParentPath(timeLineFile);
        std::string parentSec = GetParentPath(parent);
        std::vector<std::string> folders;
        std::vector<std::string> files;
        if (!FileUtil::FindFolders(parentSec, folders, files)) {
            return timeLineFile;
        }
        // 从profiler_info_文件获取
        for (const auto &file: files) {
            std::regex rankIdFileRegex(PROFILER_INFO_FILE_REG);
            if (std::regex_match(file, rankIdFileRegex)) {
                int index = file.find_last_of('_');
                int index2 = file.find_last_of('.');
                return file.substr(index + 1, index2 - index - 1);
            }
        }
        // 取上上层目录名
        std::string rankId = GetRankIdFromPath(timeLineFile);
        // 上上层目录名没有则取文件的名字
        if (rankId.empty()) {
            size_t index3 = timeLineFile.find_last_of('.');
            rankId = timeLineFile.substr(parent.length() + 1, index3 - parent.length() - 1);
        }
        return rankId;
    }

    static std::string GetRankIdFromPath(const std::string &filePath)
    {
        const int fileIdPosition = 3; // 上上层目录
        std::string path = FileUtil::GetRealPath(filePath);
        if (path.empty()) {
            return "";
        }
        auto pos = path.find_first_of('\\');
        while (pos != std::string::npos) {
            path.replace(pos, 1, "/");
            pos = path.find_first_of('\\');
        }
        auto list = StringUtil::Split(path, "/");
        if (list.size() < fileIdPosition) {
            return "";
        }
        return list.at(list.size() - fileIdPosition);
    }

    static std::string GetProfilerFileId(const std::string &filePath);

    static std::string GetDbPath(const std::string &filePath);

    static std::string GetDbPath(const std::string &filePath, const std::string &fileId);

    static inline std::string GetParentPath(const std::string filePath)
    {
        if (filePath.empty()) {
            return "";
        }
#ifdef _WIN32
        size_t pos = filePath.find_last_of("\\");
        size_t pos2 = filePath.find_last_of("\\/");
        if (pos < pos2) {
            pos = pos2;
        }
#else
        size_t pos = filePath.find_last_of("\\/");
#endif
        if (pos != std::string::npos) {
            return filePath.substr(0, pos);
        }
        return "";
    }

    static inline std::string PathPreprocess(const std::string filePath)
    {
        std::string path(filePath);
        return path;
    }

    static long long GetFileSize(const char* fileName);

    static void CalculateDirSize(const std::string &path, long long &size, int depth);

    static void RecursionFindFilesByRegex(std::vector<std::string> &result, const std::string &path, int depth,
                                          const std::regex &fileRegex);
    static std::vector<std::string> FindAllFilesByRegex(const std::string &path, const std::regex &fileRegex);

    // 遍历目录，最大不超过5层，优先遍历ASCEND_PROFILER_OUTPUT/mindstudio_profiler_output目录，其存在则跳过其他文件夹
    static inline std::vector<std::string> FindFilesWithFilter(const std::string &path, const std::regex &fileRegex)
    {
        std::vector<std::string> matchedFiles = {};
        if (!FileUtil::IsFolder(path)) {
            if (std::regex_match(FileUtil::GetFileName(path), fileRegex)) {
                matchedFiles.emplace_back(path);
            }
            return matchedFiles;
        }
        std::string error;
        std::function<void(const std::string &, int)> find = [&find, &matchedFiles, &fileRegex,
            &error](const std::string &path, int depth) {
            if (!std::empty(error)) {
                return;
            }
            if (!IsWithinRecursionLimit(matchedFiles, depth, error)) {
                return;
            }
            std::vector<std::string> folders;
            std::vector<std::string> files;
            if (!FileUtil::FindFolders(path, folders, files)) {
                return;
            }

            auto dirs = {FileUtil::SplicePath(path, ASCEND_PROFILER_OUTPUT),
                         FileUtil::SplicePath(path, MINDSTUDIO_PROFILER_OUTPUT)};
            for (const auto &dir: dirs) {
                if (FileUtil::IsFolder(dir)) {
                    find(dir, depth + 1);
                    return;
                }
            }
            for (const auto &folder: folders) {
                std::string tmpPath = FileUtil::SplicePath(path, folder);
                find(tmpPath, depth + 1);
            }
            CollectMatchdFiles(fileRegex, matchedFiles, path, files);
        };
        find(path, 0);
        if (!std::empty(error)) {
            Server::ServerLog::Warn(StringUtil::GetPrintAbleString(path), " warn is: ", error);
        }
        return matchedFiles;
    }

    static void CollectMatchdFiles(const std::regex &fileRegex, std::vector<std::string> &matchedFiles,
        const std::string &path, std::vector<std::string> &files)
    {
        sort(files.begin(), files.end(), std::greater<std::string>());
        for (const auto &file : files) {
            std::string tmpPath = SplicePath(path, file);
            if (!std::regex_match(file, fileRegex)) {
                continue;
            }
            matchedFiles.push_back(tmpPath);
            if (!RegexUtil::RegexSearch(file, SLICE_STR).has_value()) {
                // 对于分片文件，需要找到所有的带有slice的文件；其他文件，则只找最新的一个
                break;
            }
        }
    }

    static inline bool IsWithinRecursionLimit(const std::vector<std::string> &files, int depth, std::string &error)
    {
        const int depthLimit = 8;
        const size_t fileSizeLimit = 100000;
        if (depth > depthLimit) {
            error = "Too many levels of file nesting";
            return false;
        }
        if (files.size() > fileSizeLimit) {
            error = "Too many files matching the requirements";
            return false;
        }
        return true;
    }

    // 寻找目录下的第一份符合条件的文件
    static inline std::vector<std::string> FindFirstFileByRegex(const std::string &path, const std::regex &fileRegex)
    {
        std::vector<std::string> matchedFiles;
        if (!FileUtil::IsFolder(path)) {
            if (std::regex_match(FileUtil::GetFileName(path), fileRegex)) {
                matchedFiles.emplace_back(FileUtil::PathPreprocess(path));
            }
            return matchedFiles;
        }
        return FileUtil::FindFirstByRegex(path, 0, fileRegex);
    }

    inline static std::string StemFile(const std::string& fileName)
    {
        if (fileName.empty()) {
            return "";
        }
        return fileName.substr(0, fileName.find_last_of('.'));
    }

    // 切分路径
    static std::vector<std::string> SplitFilePath(std::string &path);
    static bool IsSoftLink(const std::string &path);
    static bool IsAbsolutePath(const std::string &path);
    static bool IsRegularFile(const std::string &filePath);
    static bool CheckDirValid(const std::string &path);
    static bool CheckFileValid(const std::string &filePath);
    static bool CheckFilePathExist(const std::string& filePath);
    static bool CheckFilePath(const std::string& filePath);
    static bool CheckFilePathLength(const std::string& filePath);
    static uint32_t GetFilePathLengthLimit();
    static std::string GetAbsPath(const std::string &path);
    static std::string GetCurrPath();
    static std::string GetRootPath(const std::string& filePath);
    static std::shared_ptr<std::string> GetRelativePath(const std::string& targetFilePath,
                                                        const std::string& sourceFilePath);
    static bool ModifyFilePermissions(const std::string &filePath, const mode_t &mode);
    static bool ConvertToRealPath(std::string &errorMsg, std::vector<std::string> &path);
    static bool ConvertToRealPath(std::string &errorMsg, std::string &path);
    // 寻找目录下的第一份符合条件的Db或Json数据，Db: true Json: false，集群数据仅需找到一份文件来判断
    static bool FindIfDbTypeByRegex(const std::string &path, const std::regex &jsonRegex,
                                    const std::regex &dbRegex);
    static int FindDbOrJsonType(const std::string &path, int depth,
                                const std::regex &jsonRegex, const std::regex &dbRegex);
    static std::vector<std::string> FindFirstByRegex(const std::string &path, int depth, const std::regex &fileRegex);

    static bool CheckFileSize(const std::string &filePath);

    /**
    * @brief 检查路径中是否包含非法字符
    */
    static bool CheckPathInvalidChar(const std::string &filePath);

    /**
    * @brief 检查文件属主
    */
    static bool CheckPathOwner(const std::string &filePath);

    /**
    * @brief 检查文件权限，后续替换现有实现
    */
    static bool CheckPathPermission(const std::string &filePath, fs::perms permission);
};
} // end of namespace Dic
#endif // DATA_INSIGHT_CORE_FILEUTIL_H
