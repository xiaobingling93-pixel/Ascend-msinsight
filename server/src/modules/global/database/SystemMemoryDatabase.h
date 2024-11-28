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
    std::vector<ProjectExplorerInfo> QueryProjectExplorerData(const std::vector<std::string> &projectNameList,
                                                              const std::vector<std::string>& fileNameList);
    bool InsertDuplicateUpdateProject(std::vector<ProjectExplorerInfo> projectExplorerInfos);
    bool InsertDuplicateUpdateParsedFile(std::vector<ParseFileInfo> ParseFileInfoList);
    bool UpdateProjectName(const std::string &oldProjectName, const std::string &newProjectName);
    bool UpdateProjectDbPath(const std::string &projectName, const std::string &fileName, const std::string &dbPath);
    bool DropTable();
    bool DeleteFileMenu(const std::vector<std::string> &projectNameList, const std::vector<std::string> &fileNameList);
    bool DeleteParsedFile(const std::vector<int64_t> &projectIdList, const std::vector<int64_t> &idList);
    std::map<int64_t, std::vector<ParseFileInfo>> QueryParseFileInfo(const std::vector<int64_t>& projectExplorerIdList,
                                                                     const std::vector<std::string>& parsePathList);
private:
    const std::string projectExplorerTable = "project_explorer";
    const std::string parseFileInfoTable = "parse_file_info";
};
}
}
}

#endif // PROFILER_SERVER_SYSTEMMEMORYDATABASE_H
