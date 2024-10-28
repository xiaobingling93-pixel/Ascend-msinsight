/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "FileUtil.h"
#include "pch.h"
#include "WsSessionManager.h"
#include "ProjectExplorerManager.h"
#include "CheckProjectValidHandler.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Global;

namespace {
constexpr unsigned long long CSV_SIZE = 2ULL * 1024 * 1024 * 1024;
constexpr unsigned long long JSON_AND_BIN_SIZE = 10ULL * 1024 * 1024 * 1024;
constexpr int MAX_DEPTH = 8;
std::unordered_map<std::string, unsigned long long> FILE_MAX_SIZE = {
    {".csv", CSV_SIZE},
    {"json", JSON_AND_BIN_SIZE},
    {".bin", JSON_AND_BIN_SIZE}
};
}

bool Dic::Module::CheckProjectValidHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    auto &request = dynamic_cast<ProjectCheckValidRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<ProjectCheckValidResponse> responsePtr =
            std::make_unique<ProjectCheckValidResponse>();
    ProjectCheckValidResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    ProjectErrorType error = ProjectErrorType::NO_ERRORS;
    if (!CheckRequestParamsValid(request.params, error)) {
        response.body.result = static_cast<int>(error);
    }
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}

bool Dic::Module::CheckProjectValidHandler::CheckRequestParamsValid(ProjectCheckParams &params, ProjectErrorType &error)
{
    std::string errorMsg;
    if (!params.ConvertToRealPath(errorMsg)) {
        error = ProjectErrorType::IS_UNSAFE_PATH;
        return false;
    }
    if (!CheckProjectFileSize(params.dataPath)) {
        error = ProjectErrorType::EXISTING_LARGE_FILES;
        return false;
    }

    error = ProjectExplorerManager::Instance().CheckProjectConflict(params.projectName,
        params.dataPath);
    if (error != ProjectErrorType::NO_ERRORS) {
        return false;
    }
    return true;
}

bool Dic::Module::CheckProjectValidHandler::IsFile(const std::string& path)
{
#ifdef _WIN32
    std::string tmpPath = FileUtil::PathPreprocess(path);
    DWORD attrib = GetFileAttributes(tmpPath.c_str());
    return (attrib != INVALID_FILE_ATTRIBUTES) && !(attrib & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == 0) {
        return S_ISREG(fileStat.st_mode); // Check if it's a regular file
    }
    return false; // Error or not a regular file
#endif
}

bool Dic::Module::CheckProjectValidHandler::CheckProjectFileSize(const std::vector<std::string>& filePathList)
{
    for (const auto& directoryPath : filePathList) {
        if (IsFile(directoryPath)) {
            if (!CheckFileSize(directoryPath)) {
                return false;
            }
            continue;
        }
        if (!TraverseFolder(directoryPath, MAX_DEPTH)) {
            return false;
        }
    }
    return true;
}

std::string Dic::Module::CheckProjectValidHandler::GetFileExtension(const std::string& filePath)
{
    if (filePath.size() < 4) { // 4表示文件名的最后四个字符
        return "";
    }
    std::string extension = filePath.substr(filePath.size() - 4);
    if (extension == ".csv" || extension == ".bin") {
        return extension;
    } else if (extension == "json" && filePath.size() >= 5 && filePath[filePath.length() - 5] == '.') { // 5表示取后缀
        return extension;
    }
    return "";
}

bool Dic::Module::CheckProjectValidHandler::CheckFileSize(const std::string& filePath)
{
    const std::string sufix = GetFileExtension(filePath);
    if (sufix == "") {
        return true;
    }
#ifdef _WIN32
    std::string tmpFilePath = FileUtil::PathPreprocess(filePath);
    WIN32_FILE_ATTRIBUTE_DATA fileData;
    if (GetFileAttributesEx(tmpFilePath.c_str(), GetFileExInfoStandard, &fileData)) {
        // 获取文件大小
        uintmax_t fileSize = (static_cast<uintmax_t>(fileData.nFileSizeHigh) << 32) | fileData.nFileSizeLow;
        if (fileSize > FILE_MAX_SIZE[sufix]) {
            return false;
        }
    }
#else
    // 获取文件大小
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) == 0) {
        if (fileStat.st_size > FILE_MAX_SIZE[sufix]) {
            return false;
        }
    }
#endif
    return true;
}

bool Dic::Module::CheckProjectValidHandler::TraverseFolder(const std::string& folderPath, int depth)
{
    if (depth <= 0) {
        return true;
    }
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    std::string tmpFolderPath = FileUtil::PathPreprocess(folderPath + "\\*");
    HANDLE hFind = FindFirstFile(tmpFolderPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        Server::ServerLog::Warn("Unable to open folder.");
        return true;
    }
    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if (!CheckFileSize(folderPath + "\\" + findData.cFileName)) {
                FindClose(hFind);
                return false;
            }
        } else if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
            if (!TraverseFolder(folderPath + "\\" + findData.cFileName, depth - 1)) {
                FindClose(hFind);
                return false;
            }
        }
    } while (FindNextFile(hFind, &findData) != 0);
    FindClose(hFind);
#else
    DIR* dir = opendir(folderPath.c_str());
    if (dir == nullptr) {
        Server::ServerLog::Warn("Unable to open folder.", folderPath);
        return true;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) {
            // 处理文件
            if (!CheckFileSize(folderPath + "/" + entry->d_name)) {
                closedir(dir);
                return false;
            }
        } else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // 递归处理子文件夹
            if (!TraverseFolder(folderPath + "/" + entry->d_name, depth - 1)) {
                closedir(dir);
                return false;
            }
        }
    }
    closedir(dir);
#endif
    return true;
}

}
}
