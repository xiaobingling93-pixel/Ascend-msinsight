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
    bool SaveProjectExplorer(const ProjectExplorerInfo &projectExplorerInfo, bool isConflict);
    bool DeleteProjectAndFilePath(const std::string &projectName, const std::vector<std::string>& filePathList);
    Dic::Protocol::ProjectErrorType CheckProjectConflict(const std::string &projectName,
                                                         const std::string &filePath);
    void UpdateProjectDbPath(const std::string &projectName,
                             const std::map<std::string, std::vector<std::string>>& dataPathToDbMap);
    void InitSystemMemoryDbPath(const std::string &filePath);
    bool IsClusterData(const std::string &projectName);
    bool ClearProjectExplorer(const std::vector<std::string> &projectNameList);
    static ProjectTypeEnum GetProjectType(const std::vector<ProjectExplorerInfo> &projectInfo);
    static std::vector<std::shared_ptr<ParseFileInfo>> GetClusterFilePath(
        const std::vector<ProjectExplorerInfo> &projectInfo);

private:
    std::string systemMemoryDbPath;
    std::recursive_mutex mutex;
    std::unique_ptr<Global::SystemMemoryDatabase> db;

    bool InitSystemMemoryDb();
    bool SaveProjectExplorerToDb(const std::string &projectName,
                                 const ProjectExplorerInfo &projectExplorerInfo);
    static void RebuildParseFileInfo(ProjectExplorerInfo& projectInfo,
                                     std::vector<std::shared_ptr<ParseFileInfo>> &parseFileInfos);
    std::string eventDir;
};
}
}
}
#endif // PROFILER_SERVER_FILEMENUMANAGER_H
