/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_FILEUTIL_H
#define DATA_INSIGHT_CORE_FILEUTIL_H

#include <string>
#include <dirent.h>
#include <vector>
#include "StringUtil.h"
#include <sys/stat.h>
#include "regex"
#include "RegexUtil.h"
#include "ServerLog.h"
#include "ExecUtil.h"
#include "FileDef.h"
#include <fstream>

#if defined(_WIN32)

#include <windows.h>
#include <shlwapi.h>
#include <io.h>
#include <tchar.h>

#else
#include <sys/stat.h>
#include <unistd.h>
#include <climits>
#endif
#ifdef __APPLE__
#include <filesystem>
#endif

namespace Dic {
class FileUtil {
public:
    static inline bool CheckDirectoryExist(const std::string &path)
    {
#ifdef _WIN32
        std::string tmpPath(path);
        if (StringUtil::IsUtf8String(path)) {
            tmpPath = StringUtil::Utf8ToGbk(path.c_str());
        }
        if (_access(tmpPath.c_str(), 0) == -1) {
            return false;
        }
#else
        if (access(path.c_str(), 0) == -1) {
            return false;
        }
#endif
        return true;
    }

    static inline std::string SplicePath(const std::string &path, const std::string &fileName)
    {
        if (path.empty()) {
            return fileName;
        }
        std::string res(path);
#ifdef _WIN32
        if (path[path.size() - 1] != '/' && path[path.size() - 1] != '\\') {
            res.append("\\");
        }
#else
        if (path[path.size() - 1] != '/') {
            res.append("/");
        }
#endif
        res.append(fileName);
        return res;
    }

    static inline std::string GetRealPath(const std::string &path)
    {
#ifdef _WIN32
        char resolvedPath[MAX_PATH] = {0};
        _fullpath(resolvedPath, path.c_str(), MAX_PATH);
#else
        char resolvedPath[PATH_MAX] = {0};
        realpath(path.c_str(), resolvedPath);
#endif
        return std::string(resolvedPath);
    }

    static inline uint64_t GetDiskFreeSize(const std::string &path)
    {
#ifdef _WIN32
        std::string tmpPath(path);
        if (StringUtil::IsUtf8String(path)) {
            tmpPath = StringUtil::Utf8ToGbk(path.c_str());
        }
        ULARGE_INTEGER freeBytes;
        if (GetDiskFreeSpaceEx(tmpPath.c_str(), &freeBytes, nullptr, nullptr)) {
            return freeBytes.QuadPart;
        }
#endif
#ifdef __APPLE__
        return std::filesystem::space(path).available;
#endif
        return 0;
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
                                   std::vector<std::string> &folders,
                                   std::vector<std::string> &files)
    {
        long hFile = 0;
        struct _finddata_t fileInfo{};
        std::string tmpPath;
        if (StringUtil::IsUtf8String(path)) {
            tmpPath = StringUtil::Utf8ToGbk(path.c_str());
        }
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
            if ((fileInfo.attrib & _A_SUBDIR) != 0) {
                folders.emplace_back(StringUtil::GbkToUtf8(fileInfo.name));
            } else {
                files.emplace_back(StringUtil::GbkToUtf8(fileInfo.name));
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
        std::string tmpPath(path);
        pDir = opendir(tmpPath.c_str());
        if (pDir == nullptr) {
            return false;
        }
        while ((pDirent = readdir(pDir)) != nullptr) {
            if (std::string(pDirent->d_name) == ".." || std::string(pDirent->d_name) == ".") {
                continue;
            }
            if (pDirent->d_type == DT_DIR) {
                folders.emplace_back(pDirent->d_name);
            } else if (pDirent->d_type == DT_REG) {
                files.emplace_back(pDirent->d_name);
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
        if (StringUtil::IsUtf8String(path)) {
            tmpPath = StringUtil::Utf8ToGbk(path.c_str());
        }
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
#ifdef _WIN32
        if (StringUtil::IsUtf8String(path)) {
            tmpPath = StringUtil::Utf8ToGbk(path.c_str());
        }
#endif
        return std::remove(tmpPath.c_str()) == 0;
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

    static inline bool IsHiddenPath(std::string_view path)
    {
        if (path.empty()) {
            return false;
        }
#ifdef _WIN32
        std::string tmpPath(path);
        if (StringUtil::IsUtf8String(path.data())) {
            tmpPath = StringUtil::Utf8ToGbk(path.data());
        }
        auto attribute = GetFileAttributes(tmpPath.data());
        return attribute == INVALID_FILE_ATTRIBUTES || (attribute & FILE_ATTRIBUTE_HIDDEN) != 0;
#else
        return path[0] == '.';
#endif
    }

    static inline std::map<std::string, std::vector<std::string>>
    SplitToRankList(std::vector<std::pair<std::string, std::string>> fileList)
    {
        std::map<std::string, std::vector<std::string>> rankListMap;
        for (const auto &item: fileList) {
            std::string rankId = item.second;
            rankListMap[rankId].push_back(item.first);
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
            std::regex rankIdFileRegex("profiler_info_[0-9]{1,5}.json");
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
            int index3 = timeLineFile.find_last_of('.');
            rankId = timeLineFile.substr(parent.length() + 1, index3 - parent.length() - 1);
        }
        return rankId;
    }

    static std::string GetRankIdFromPath(const std::string &filePath)
    {
        const int fileIdPosition = 3; // 上上层目录
        std::string path = FileUtil::GetRealPath(filePath);
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

    static inline std::string GetParentPath(const std::string filePath)
    {
        size_t pos = filePath.find_last_of("\\/");
        if (pos != std::string::npos) {
            return filePath.substr(0, pos);
        }
        return "";
    }

    static inline std::string PathPreprocess(const std::string filePath)
    {
        std::string path(filePath);
#ifdef _WIN32
        if (StringUtil::IsUtf8String(filePath)) {
            path = StringUtil::Utf8ToGbk(filePath.c_str());
        }
#endif
        return path;
    }

    static inline std::string GetDetailFile(std::string parentDir, std::string detailName)
    {
        std::string path = PathPreprocess(SplicePath(parentDir, detailName));
        std::ifstream file(path);
        if (file.good()) {
            return path;
        }
        return "";
    }

    static long getFileSize(const char* fileName)
    {
        FILE *fp = fopen(fileName, "r");
        fseek(fp, 0L, SEEK_END);
        long size = ftell(fp);
        fclose(fp);
        return size;
    }

    static void CalculateDirSize(const std::string &path, long long &size, int depth)
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
            if (std::strcmp("trace_view.db", file.c_str()) != 0 &&
                    std::strcmp("cluster.db", file.c_str()) != 0) {
                size +=  getFileSize(spliceFile.c_str());
            }
        }
    }

    static inline std::vector<std::string> FindFilesByRegex(const std::string &path, const std::regex &fileRegex)
    {
        std::vector<std::string> matchedFiles;
        if (!FileUtil::IsFolder(path)) {
            matchedFiles.emplace_back(PathPreprocess(path));
            return matchedFiles;
        }
        std::function<void(const std::string &, int)> find = [&find, &matchedFiles, &fileRegex](
                const std::string &path, int depth) {
            if (depth > 5) {
                return;
            }
            std::vector<std::string> folders;
            std::vector<std::string> files;
            if (!FileUtil::FindFolders(path, folders, files)) {
                return;
            }
            for (const auto &folder: folders) {
                std::string tmpPath = FileUtil::SplicePath(path, folder);
                find(tmpPath, depth + 1);
            }
            for (const auto &file: files) {
                std::string tmpPath = FileUtil::SplicePath(path, file);
                if (std::regex_match(file, fileRegex)) {
                    matchedFiles.push_back(PathPreprocess(tmpPath));
                }
            }
        };
        find(path, 0);
        return matchedFiles;
    }

    static std::string GetDbPath(const std::string &filePath, const std::string &fileId)
    {
        std::string dbName = fileId + ".db";
        std::string dbPath = SplicePath(filePath, dbName);
        return Dic::FileUtil::GetRealPath(dbPath);
    }

    static std::vector<std::string> FindFileByName(const std::string &path,
                                                   const std::string &fileName, const std::string &fileReg)
    {
        std::vector<std::string> files;
        if (!FileUtil::IsFolder(path)) {
            files.emplace_back(path);
            return files;
        }
        std::function<void(const std::string &, int)> find = [&find, &files, fileReg, fileName](
                const std::string &path, int depth) {
            if (depth > 5) {
                return;
            }
            std::vector<std::string> folders;
            std::vector<std::string> fileList;
            if (!FileUtil::FindFolders(path, folders, fileList)) {
                return;
            }
            if (std::find(folders.begin(), folders.end(), "ASCEND_PROFILER_OUTPUT") != folders.end()) {
                FindAscendFolder(path, files, fileName, fileReg);
                return;
            }
            for (const auto &folder: folders) {
                std::string tmpPath = FileUtil::SplicePath(path, folder);
                find(tmpPath, depth + 1);
            }
            for (const auto &file: fileList) {
                std::string tmpPath = FileUtil::SplicePath(path, file);
                if (IsFileValid(file, fileReg)) {
                    files.push_back(tmpPath);
                }
            }
        };
        find(path, 0);
        return files;
    }

    static bool IsFileValid(const std::string &fileName, const std::string &fileReg)
    {
        auto result = RegexUtil::RegexMatch(fileName, fileReg);
        return result.has_value();
    }

    static void FindAscendFolder(const std::string &path, std::vector<std::string> &ascendFiles,
                                 const std::string &fileName, const std::string &fileReg)
    {
        std::string splicePath = FileUtil::SplicePath(path, "ASCEND_PROFILER_OUTPUT");
        splicePath = FileUtil::SplicePath(splicePath, fileName);
        Server::ServerLog::Info("FindAscendFolder. ", splicePath);
        if (FileUtil::CheckDirectoryExist(splicePath)) {
            ascendFiles.emplace_back(splicePath);
            Server::ServerLog::Info("FindAscendFolder2. ");
            return;
        }
        std::function<void(const std::string &, int)> find = [&find, &ascendFiles, fileReg](
                const std::string &path, int depth) {
            if (depth > 5) {
                return;
            }
            std::vector<std::string> folders;
            std::vector<std::string> files;
            if (!FileUtil::FindFolders(path, folders, files)) {
                return;
            }
            for (const auto &folder: folders) {
                find(FileUtil::SplicePath(path, folder), depth + 1);
            }
            for (const auto &file: files) {
                if (IsFileValid(file, fileReg)) {
                    ascendFiles.push_back(FileUtil::SplicePath(path, file));
                }
            }
        };
        std::vector<std::string> folders;
        std::vector<std::string> files;
        if (!FileUtil::FindFolders(path, folders, files)) {
            return;
        }
        static std::string reg = R"(PROF_.*)";
        for (const auto &folder: folders) {
            if (!RegexUtil::RegexMatch(folder, reg).has_value()) {
                continue;
            }
            find(FileUtil::SplicePath(path, folder), 0);
            break;
        }
    }

    static bool CheckFilePath(std::string filePath)
    {
        // reading
        if (access(filePath.c_str(), R_OK) == -1) {
            Server::ServerLog::Error("Cannot read" + filePath +": ", filePath);
            return false;
        }
        // isOk
        std::ifstream file(filePath);
        if (!file.good()) {
            Server::ServerLog::Error("Cannot get" + filePath +": ", filePath);
            return false;
        }
        // size checked
        long long size = getFileSize(filePath.c_str());
        if (size >= MAX_FILE_SIZE_2G) {
            Server::ServerLog::Error("The size of " + filePath + " is too large in path:", filePath);
            return false;
        }
        return true;
    }

    static std::string GetCurrPath()
    {
        char currPath[1024];
    #ifdef _WIN32
            ::GetModuleFileName(NULL, currPath, MAX_PATH);
            (_tcsrchr(currPath, '\\'))[1] = 0;
    #else
            getcwd(currPath, 1024);
      sprintf(currPath, "%s/", currPath);
    #endif
        std::string strCurrPath = currPath;
        return strCurrPath;
    }
};
} // end of namespace Dic
#endif // DATA_INSIGHT_CORE_FILEUTIL_H
