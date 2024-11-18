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
constexpr uint64_t FILE_COUNT_LIMIT = 100000; // 最大遍历文件数量
using ErrorType = ProjectErrorType;
namespace Module = Dic::Module;
std::unordered_map<std::string, unsigned long long> FILE_MAX_SIZE = {
    {".csv", CSV_SIZE},
    {".json", JSON_AND_BIN_SIZE},
    {".bin", JSON_AND_BIN_SIZE},
    {".db", JSON_AND_BIN_SIZE}
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
    uint64_t fileCount = 0;
    for (const auto &path: params.dataPath) {
        if (++fileCount > FILE_COUNT_LIMIT) {
            break;
        }
        if (!fs::exists(path)) {
            continue;
        }
        if (fs::is_directory(path)) {
            if (TraverseFolder(params, path, fileCount, error)) {
                continue;
            }
            return false;
        }
        if (!CheckProjectFile(params, fs::path(path), error)) {
            return false;
        }
    }
    error = ProjectExplorerManager::Instance().CheckProjectConflict(params.projectName, params.dataPath);
    if (error != ProjectErrorType::NO_ERRORS) {
        return false;
    }
    return true;
}

bool Dic::Module::CheckProjectValidHandler::CheckProjectFile(ProjectCheckParams &params, const fs::path &filePath,
                                                             ProjectErrorType &error)
{
    if (FILE_MAX_SIZE.count(filePath.extension().string()) == 0) {
        return true;
    }
    if (!CheckFileSize(filePath)) {
        error = ProjectErrorType::EXISTING_LARGE_FILES;
        return false;
    }
    return true;
}

bool Dic::Module::CheckProjectValidHandler::CheckFileSize(const fs::path &filePath)
{
    if (fs::file_size(filePath) > FILE_MAX_SIZE[filePath.extension().string()]) {
        return false;
    }
    return true;
}

bool CheckProjectValidHandler::TraverseFolder(ProjectCheckParams &params, const std::string& folderPath,
                                              uint64_t &fileCount, ProjectErrorType &error)
{
    for (auto &file : fs::directory_iterator(folderPath)) {
        if (++fileCount > FILE_COUNT_LIMIT) {
            break;
        }
        auto path = file.path().string();
        if (fs::is_directory(file)) {
            if (TraverseFolder(params, path, fileCount, error)) {
                continue;
            }
            return false;
        }
        if (!CheckProjectFile(params, path, error)) {
            return false;
        }
    }
    return true;
}

}
}
