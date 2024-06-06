/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SYSTEMMEMORYDATABASE_H
#define PROFILER_SERVER_SYSTEMMEMORYDATABASE_H

#include "Database.h"
#include "SystemMemoryDatabaseDef.h"
#include "vector"

namespace Dic {
namespace Module {
namespace Global {
class SystemMemoryDatabase : public Database {
public:
    explicit SystemMemoryDatabase(std::recursive_mutex &sqlMutex) : Database(sqlMutex) {};
    ~SystemMemoryDatabase() override = default;

    bool SetConfig();
    bool CreateTable();
    std::vector<ProjectExplorerInfo> QueryProjectExplorerData(const std::string &projectName,
                                                              const std::vector<std::string>& fileNameList);
    bool SaveProjectExplorerData(ProjectExplorerInfo projectExplorerInfo);
    bool UpdateProjectName(const std::string &oldProjectName, const std::string &newProjectName);
    bool UpdateProjectDbPath(const std::string &projectName, const std::string &fileName, const std::string &dbPath);
    bool DropTable();
    bool DeleteFileMenu(const std::string &projectName, const std::vector<std::string>& fileNameList);
private:
    const std::string projectExplorerTable = "project_explorer";
};
}
}
}

#endif // PROFILER_SERVER_SYSTEMMEMORYDATABASE_H
