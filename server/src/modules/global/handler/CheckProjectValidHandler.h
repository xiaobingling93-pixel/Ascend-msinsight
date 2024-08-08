/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_CHECKPROJECTVALIDHANDLER_H
#define PROFILER_SERVER_CHECKPROJECTVALIDHANDLER_H

#include <vector>
#include <string>
#include <dirent.h>
#include <sys/stat.h> // 包含 struct stat

#include "GlobalHandler.h"
#include "GlobalProtocolRequest.h"

namespace Dic {
namespace Module {

class CheckProjectValidHandler : public GlobalHandler {
public:
    CheckProjectValidHandler()
    {
        command = REQ_RES_PROJECT_VALID_CHECK;
    }
    ~CheckProjectValidHandler() override = default;

    void HandleRequest(std::unique_ptr<Request> requestPtr) override;
private:
    static bool CheckRequestParamsValid(ProjectCheckParams &params, ProjectErrorType &error);
    static bool CheckProjectFileSize(const std::vector<std::string>& filePathList);
    static std::string GetFileExtension(const std::string& filePath);
    static bool CheckFileSize(const std::string& filePath);
    static bool TraverseFolder(const std::string& folderPath, int depth);
    static bool IsFile(const std::string& path);
};
} // end of namespace Module
} // Dic

#endif // PROFILER_SERVER_CHECKPROJECTVALIDHANDLER_H
