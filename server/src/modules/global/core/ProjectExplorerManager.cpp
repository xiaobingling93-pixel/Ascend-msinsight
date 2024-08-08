/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "SystemMemoryDatabase.h"
#include "SystemMemoryDatabaseDef.h"
#include "ParserFactory.h"
#include "ProjectExplorerManager.h"

namespace Dic {
namespace Module {
namespace Global {

ProjectExplorerManager &ProjectExplorerManager::Instance()
{
    static ProjectExplorerManager instance;
    return instance;
}

bool ProjectExplorerManager::UpdateProjectName(const std::string& oldProjectName, const std::string& newProjectName)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (!InitSystemMemoryDb()) {
        Server::ServerLog::Error("Failed to open database. path:", systemMemoryDbPath);
        return false;
    }
    if (!db->UpdateProjectName(oldProjectName, newProjectName)) {
        return false;
    }
    return true;
}

std::vector<ProjectExplorerInfo> ProjectExplorerManager::QueryProjectExplorer(
    const std::string &projectName, const std::vector<std::string> &dataPathList)
{
    if (!InitSystemMemoryDb()) {
        Server::ServerLog::Error("Failed to open database. path:", systemMemoryDbPath);
        return {};
    }
    return db->QueryProjectExplorerData(projectName, dataPathList);
}

bool ProjectExplorerManager::SaveProjectExplorer(const std::string& projectName, const std::string& filePath,
                                                 ProjectTypeEnum projectType, const std::string& importType,
                                                 const std::vector<std::string> &dbPath)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (!InitSystemMemoryDb()) {
        Server::ServerLog::Error("Failed to open database. path:", systemMemoryDbPath);
        return false;
    }
    db->StartTransaction();
    // 查询该项目下的所有文件名
    std::vector<ProjectExplorerInfo> infos =
            db->QueryProjectExplorerData(projectName, std::vector<std::string>());
    std::vector<std::string> fileNameList;
    for (const auto &item: infos) {
        fileNameList.push_back(item.fileName);
    }
    // 如果文件已存在 则直接返回
    if (std::find(fileNameList.begin(), fileNameList.end(), filePath) != fileNameList.end()) {
        Server::ServerLog::Info("Save project explorer success, file already exist:", filePath);
        db->EndTransaction();
        return true;
    }
    // 判断文件类型，如果当前文件类型与该项目下其他文件类型不一致，则需要对该项目下的数据进行清空处理
    bool isProjectTypeConflict = !infos.empty() &&
            (infos[0].projectType != static_cast<int64_t>(projectType) || infos[0].importType == "drag"
            || isCoverProjectType(projectType));
    if (isProjectTypeConflict && !db->DeleteFileMenu(projectName, std::vector<std::string>())) {
        db->RollbackTransaction();
        return false;
    }
    // 插入操作
    ProjectExplorerInfo info;
    info.projectType = static_cast<int64_t>(projectType);
    info.fileName = filePath;
    info.projectName = projectName;
    info.importType = importType;
    info.dbPath = dbPath;
    if (!db->SaveProjectExplorerData(info)) {
        db->RollbackTransaction();
        return false;
    }
    db->EndTransaction();
    return true;
}

bool ProjectExplorerManager::InitSystemMemoryDb()
{
    if (!db) {
        db = std::make_unique<SystemMemoryDatabase>(mutex);
    }
    if (db->IsOpen()) {
        return true;
    }
    if (db->OpenDb(systemMemoryDbPath, false) && db->SetConfig() && db->CreateTable()) {
        return true;
    }
    return false;
}

bool ProjectExplorerManager::DeleteProjectAndFilePath(const std::string &projectName,
                                                      const std::vector<std::string>& filePathList)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (projectName.empty() && filePathList.empty()) {
        return true;
    }
    if (!InitSystemMemoryDb()) {
        Server::ServerLog::Error("Failed to open database. path:", systemMemoryDbPath);
        return false;
    }

    std::vector<ProjectExplorerInfo> infos = db->QueryProjectExplorerData(projectName, filePathList);
    for (const auto &item: infos) {
        if (item.dbPath.empty()) {
            continue;
        }
        for (const auto &dbPathItem: item.dbPath) {
            FileUtil::RemoveFileExDb(dbPathItem);
        }
    }

    if (!db->DeleteFileMenu(projectName, filePathList)) {
        return false;
    }
    return true;
}

bool ProjectExplorerManager::CheckProjectConflict(const std::string &projectName,
                                                  const std::vector<std::string>& filePathList)
{
    if (projectName.empty() || filePathList.empty()) {
        return false;
    }

    std::pair<std::string, ParserType> parserType = ParserFactory::GetImportType(filePathList);
    ParserType allocType = parserType.second;
    std::shared_ptr<ParserAlloc> factory = ParserFactory::ParserImport(allocType);
    ProjectTypeEnum projectTypeEnum = factory->GetProjectType(filePathList);

    if (!InitSystemMemoryDb()) {
        Server::ServerLog::Error("Failed to open database. path:", systemMemoryDbPath);
        return false;
    }
    std::vector<ProjectExplorerInfo> infos =
            db->QueryProjectExplorerData(projectName, std::vector<std::string>());

    bool isConflict = !infos.empty() &&
            (infos[0].projectType != static_cast<int64_t>(projectTypeEnum) || infos[0].importType == "drag"
            || isCoverProjectType(projectTypeEnum));
    return isConflict;
}

void ProjectExplorerManager::UpdateProjectDbPath(const std::string &projectName,
                                                 const std::map<std::string, std::vector<std::string>>& dataPathToDbMap)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (projectName.empty()) {
        return;
    }
    if (!InitSystemMemoryDb()) {
        Server::ServerLog::Error("Failed to open database. path:", systemMemoryDbPath);
        return;
    }
    db->StartTransaction();
    for (auto &it : dataPathToDbMap) {
        std::string dbPathStr = StringUtil::join(it.second, ",");
        db->UpdateProjectDbPath(projectName, it.first, dbPathStr);
    }
    db->EndTransaction();
}

void ProjectExplorerManager::InitSystemMemoryDbPath(const std::string &filePath)
{
    std::string path = FileUtil::SplicePath(filePath, "system_memory.db");
#ifdef _WIN32
    systemMemoryDbPath = StringUtil::GbkToUtf8(path.c_str());
#else
    systemMemoryDbPath = path;
#endif
}

}
}
}