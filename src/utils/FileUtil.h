/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_FILEUTIL_H
#define DATA_INSIGHT_CORE_FILEUTIL_H

#include <string>
#if defined(_WIN32)
#include <windows.h>
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

    static inline void SplicePath(std::string &path, const std::string fileName)
    {
        if (path.empty()) {
            path = fileName;
            return;
        }
#ifdef _WIN32
        if (path[path.size() - 1] != '/' || path[path.size() - 1] != '\\') {
            path += "\\";
        }
#else
        if (path[path.size() - 1] != '/') {
            path += "/";
        }
#endif
        path += fileName;
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
};
} // end of namespace Dic
#endif // DATA_INSIGHT_CORE_FILEUTIL_H
