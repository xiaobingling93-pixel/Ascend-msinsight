/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_FILEMENUMANAGER_H
#define PROFILER_SERVER_FILEMENUMANAGER_H

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include "GlobalProtocolResponse.h"
#include "SystemMemoryDatabase.h"
#include "ParamsParser.h"

namespace Dic {
namespace Module {
namespace Global {
class ProjectExplorerManager {
public:
    static ProjectExplorerManager &Instance();
    ProjectExplorerManager() = default;
    ~ProjectExplorerManager() = default;
    bool UpdateProjectName(const std::string& oldProjectName, const std::string& newProjectName);
    std::vector<ProjectExplorerInfo> QueryProjectExplorer(const std::string &projectName,
                                                          const std::vector<std::string> &dataPathList);
    bool SaveProjectExplorer(const std::string &projectName, const std::string &filePath, ProjectTypeEnum projectType,
                             const std::string& importType, const std::vector<std::string> &dbPath);
    bool DeleteProjectAndFilePath(const std::string &projectName, const std::vector<std::string>& filePathList);
    bool CheckProjectConflict(const std::string &projectName, const std::vector<std::string>& filePathList);
    void UpdateProjectDbPath(const std::string &projectName,
                             const std::map<std::string, std::vector<std::string>>& dataPathToDbMap);
    void InitSystemMemoryDbPath(const std::string &filePath);

private:
    std::string systemMemoryDbPath;
    std::recursive_mutex mutex;
    bool InitSystemMemoryDb();
    std::unique_ptr<Global::SystemMemoryDatabase> db;
};
}
}
}
#endif // PROFILER_SERVER_FILEMENUMANAGER_H
