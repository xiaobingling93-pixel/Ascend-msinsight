/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_FILEUTIL_H
#define DATA_INSIGHT_CORE_FILEUTIL_H

#include <string>
#include <dirent.h>

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
        return std::remove(path.c_str()) == 0;
    }

    static inline std::map<std::string, std::vector<std::string>>
    SplitToRankList(std::vector<std::string> fileList)
    {
        std::map<std::string, std::vector<std::string>> rankListMap;
        for (const auto &item: fileList) {
            std::string rankId = GetRankIdFromFile(item);
            rankListMap[rankId].push_back(item);
        }
        return rankListMap;
    }

    static inline std::string GetRankIdFromFile(std::string timeLineFile)
    {
        std::string parent = GetParentPath(GetParentPath(timeLineFile));
        auto folders = FileUtil::FindFolders(parent);
        for (const auto &folder: folders) {
            std::regex rankIdFileRegex("profiler_info_[0-9]{1,5}.json");
            if (std::regex_match(folder, rankIdFileRegex)) {
                int index = folder.find_last_of('_');
                int index2 = folder.find_last_of('.');
                return folder.substr(index + 1, index2 - index - 1);
            }
        }
    }

    static inline std::string GetParentPath(std::string filePath)
    {
        size_t pos = filePath.find_last_of("\\/");
        if (pos != std::string::npos) {
            return filePath.substr(0, pos);
        }
    }

    static inline std::string GetDetailFile(std::string traceViewFileName, std::string detailName)
    {
        std::string parent = GetParentPath(traceViewFileName);
        std::string path = SplicePath(parent, detailName);
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
        auto folders = FileUtil::FindFolders(path);
        for (std::string& file: folders) {
            if (FileUtil::IsFolder(file)) {
                CalculateDirSize(file, size, depth + 1);
            } else if (FileUtil::CheckDirectoryExist(file)) {
                if (!(depth == 0 && file.find_last_of(".db") > 0)) {
                    size +=  getFileSize(file.c_str());
                }
            }
        }
    }

    static inline std::vector<std::string> FindFilesByRegex(const std::string &path, const std::regex &fileRegex)
    {
        std::vector<std::string> matchedFiles;
        if (!FileUtil::IsFolder(path)) {
            matchedFiles.emplace_back(path);
            return matchedFiles;
        }
        std::function<void(const std::string &, int)> find = [&find, &matchedFiles, &fileRegex](
                const std::string &path, int depth) {
            if (depth > 5) {
                return;
            }
            auto folders = FileUtil::FindFolders(path);
            for (const auto &folder: folders) {
                std::string tmpPath = FileUtil::SplicePath(path, folder);
                if (FileUtil::IsFolder(tmpPath)) {
                    find(tmpPath, depth + 1);
                } else if (std::regex_match(folder, fileRegex)) {
                    matchedFiles.push_back(tmpPath);
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
};
} // end of namespace Dic
#endif // DATA_INSIGHT_CORE_FILEUTIL_H
