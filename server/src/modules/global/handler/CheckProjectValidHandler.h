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

    bool HandleRequest(std::unique_ptr<Request> requestPtr) override;
private:
    static bool CheckRequestParamsValid(ProjectCheckParams &params, ProjectErrorType &error);
    static bool CheckProjectFile(ProjectCheckParams &params, const fs::path &filePath, ProjectErrorType &error);
    static bool CheckFileSize(const fs::path &filePath);
    static bool TraverseFolder(ProjectCheckParams &params, const std::string& folderPath, uint64_t &fileCount,
                               ProjectErrorType &error);
};
} // end of namespace Module
} // Dic

#endif // PROFILER_SERVER_CHECKPROJECTVALIDHANDLER_H
