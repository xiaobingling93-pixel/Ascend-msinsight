/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_FILEUTIL_H
#define DATA_INSIGHT_CORE_FILEUTIL_H

#include <string>
#include <dirent.h>
#include <vector>
#if defined(_WIN32)
#include <windows.h>
#include <shlwapi.h>
#include <io.h>
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
        if (_access(path.c_str(), 0) == -1) {
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
        if (path[path.size() - 1] != '/' || path[path.size() - 1] != '\\') {
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
        ULARGE_INTEGER freeBytes;
        if (GetDiskFreeSpaceEx(path.c_str(), &freeBytes, nullptr, nullptr)) {
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

    // return file and folder in path
    static inline std::vector<std::string> FindFolders(const std::string &path)
    {
        if (path.empty()) {
            return {};
        }
        DIR *pDir = nullptr;
        struct dirent *pDirent = nullptr;
        pDir = opendir(path.c_str());
        if (pDir == nullptr) {
            return {};
        }
        std::vector<std::string> folders;
        while ((pDirent = readdir(pDir)) != nullptr) {
            if (std::string(pDirent->d_name) == ".." || std::string(pDirent->d_name) == ".") {
                continue;
            }
            folders.emplace_back(pDirent->d_name);
        }
        closedir(pDir);
        return folders;
    }

    static inline bool IsFolder(const std::string &path)
    {
#ifdef _WIN32
        return PathIsDirectory(path.c_str());
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
        return  std::remove(path.c_str()) == 0;
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
        auto attribute = GetFileAttributes(path.data());
        return (attribute & FILE_ATTRIBUTE_HIDDEN) != 0 ;
#else
        return path[0] == '.';
#endif
    }
};
} // end of namespace Dic
#endif // DATA_INSIGHT_CORE_FILEUTIL_H
