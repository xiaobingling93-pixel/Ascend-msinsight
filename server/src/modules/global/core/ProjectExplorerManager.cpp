/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <unordered_map>
#include <algorithm>
#include "ParamsParser.h"
#include "pch.h"
#include "SystemMemoryDatabase.h"
#include "SystemMemoryDatabaseDef.h"
#include "ParserFactory.h"
#include "TimeUtil.h"
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
    std::vector<std::string> projectNameList;
    if (!projectName.empty()) {
        projectNameList.push_back(projectName);
    }
    std::vector<ProjectExplorerInfo> projectExplorerList =
        db->QueryProjectExplorerData(projectNameList, std::vector<std::string>());
    std::vector<int64_t> projectExplorerIdList;
    for (const auto &item: projectExplorerList) {
        projectExplorerIdList.push_back(item.id);
    }
    std::map<int64_t, std::vector<ParseFileInfo>> parseFileInfoMap = db->QueryParseFileInfo(projectExplorerIdList,
                                                                                            dataPathList);
    std::vector<ProjectExplorerInfo> res;
    for (auto &item: projectExplorerList) {
        if (parseFileInfoMap.find(item.id) != parseFileInfoMap.end()) {
            item.parseFilePathInfos = parseFileInfoMap[item.id];
            res.push_back(item);
        }
    }
    return res;
}

bool ProjectExplorerManager::SaveProjectExplorer(std::vector<ProjectExplorerInfo> &projectExplorerInfos,
                                                 bool isConflict)
{
    // 前置校验：要保存的项目不能为空
    if (projectExplorerInfos.empty()) {
        return false;
    }
    // 前置校验：检查入参所有导入文件是否属于同一个项目下，如果不是，则直接返回
    std::string projectName = projectExplorerInfos[0].projectName;
    for (size_t i = 1; i < projectExplorerInfos.size(); ++i) {
        if (projectExplorerInfos[i].projectName != projectName) {
            return false;
        }
    }
    // db初始化
    if (!InitSystemMemoryDb()) {
        Server::ServerLog::Error("Failed to open database. path:", systemMemoryDbPath);
        return false;
    }
    std::unique_lock<std::recursive_mutex> lock(mutex);
    db->StartTransaction();
    // 如果存在冲突，则需要清空老项目内容，如果处理失败，则事务回滚
    if (isConflict && !db->DeleteFileMenu(std::vector<std::string>{projectName}, std::vector<std::string>())) {
        db->RollbackTransaction();
        return false;
    }

    // 将导入数据进行落盘，如果失败则回滚
    if (!SaveProjectExplorerToDb(projectName, projectExplorerInfos)) {
        db->RollbackTransaction();
        return false;
    }
    // 提交事务
    db->EndTransaction();
    return true;
}

bool ProjectExplorerManager::SaveProjectExplorerToDb(const std::string &projectName,
                                                     std::vector<ProjectExplorerInfo> &projectExplorerInfos)
{
    if (!db->InsertDuplicateUpdateProject(projectExplorerInfos)) {
        return false;
    }
    std::vector<ProjectExplorerInfo> projectExplorerInfoData =
            db->QueryProjectExplorerData(std::vector<std::string>{projectName}, std::vector<std::string>());
    if (projectExplorerInfoData.empty()) {
        return false;
    }
    std::unordered_map<std::string, int64_t> ukIdMap;
    for (const auto &item: projectExplorerInfoData) {
        ukIdMap[item.projectName + item.fileName] = item.id;
    }

    std::vector<ParseFileInfo> parseFileInfos;
    for (auto &project: projectExplorerInfos) {
        std::string uk = project.projectName + project.fileName;
        if (ukIdMap.find(uk) == ukIdMap.end()) {
            return false;
        }
        int64_t id = ukIdMap[project.projectName + project.fileName];
        for (auto &item: project.parseFilePathInfos) {
            item.projectExplorerId = id;
            parseFileInfos.push_back(item);
        }
    }
    return db->InsertDuplicateUpdateParsedFile(parseFileInfos);
}

bool ProjectExplorerManager::InitSystemMemoryDb()
{
    if (!db) {
        InitSystemMemoryDbPath(Server::ParamsParser::Instance().GetOption().logPath);
        db = std::make_unique<SystemMemoryDatabase>(mutex);
    }
    // 如果db已经打开，则直接返回
    if (db->IsOpen()) {
        return true;
    }
    // 打开db
    if (!db->OpenDb(systemMemoryDbPath, false)) {
        return false;
    }
    // 判断db是否发生版本变更，如果是，则进行数据表删除操作
    if (db->IsDatabaseVersionChange() && !db->DropAllTable()) {
        return false;
    }
    // db配置并创建表
    if (db->SetConfig() && db->CreateTable() && db->SetDataBaseVersion()) {
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

    // 获取项目下所有内容
    std::vector<ProjectExplorerInfo> infos = QueryProjectExplorer(projectName, std::vector<std::string>());
    if (infos.empty()) {
        return true;
    }
    std::vector<int64_t> projectIdList;
    std::vector<std::string> needDeleteImportFileList;
    std::vector<int64_t> needDeleteParseFileIdList;
    for (const auto &project: infos) {
        projectIdList.push_back(project.id);
        bool isNeedDeleteImportData = true;
        for (const auto &item: project.parseFilePathInfos) {
            if (std::find(filePathList.begin(), filePathList.end(), item.parseFilePath) != filePathList.end()) {
                needDeleteParseFileIdList.push_back(item.id);
            } else if (!filePathList.empty()) {
                // 如果出现了一个导入记录下，有文件没有被删除干净，则不删除该记录
                isNeedDeleteImportData = false;
            }
        }
        if (isNeedDeleteImportData) {
            needDeleteImportFileList.push_back(project.fileName);
        }
    }

    if (!needDeleteImportFileList.empty()) {
        db->DeleteFileMenu(std::vector<std::string>{projectName}, needDeleteImportFileList);
    }
    if (!db->DeleteParsedFile(projectIdList, needDeleteParseFileIdList)) {
        return false;
    }
    return true;
}

ProjectErrorType ProjectExplorerManager::CheckProjectConflict(const std::string &projectName,
                                                              const std::vector<std::string>& filePathList)
{
    if (projectName.empty() || filePathList.empty()) {
        return ProjectErrorType::OTHER;
    }

    std::pair<std::string, ParserType> parserType = ParserFactory::GetImportType(filePathList);
    ParserType allocType = parserType.second;
    std::shared_ptr<ParserAlloc> factory = ParserFactory::ParserImport(allocType);
    ProjectTypeEnum projectTypeEnum = factory->GetProjectType(filePathList);

    if (!InitSystemMemoryDb()) {
        Server::ServerLog::Error("Failed to open database. path:", systemMemoryDbPath);
        return ProjectErrorType::OTHER;
    }
    std::vector<ProjectExplorerInfo> infos =
            db->QueryProjectExplorerData(std::vector<std::string>{projectName}, std::vector<std::string>());

    // 校验是否导入的数据是否是历史导入过的
    std::vector<std::string> curFilePathList;
    for (const auto &item: infos) {
        curFilePathList.push_back(item.fileName);
    }
    std::vector<std::string> diff;
    std::set_difference(filePathList.begin(), filePathList.end(), curFilePathList.begin(),
                        curFilePathList.end(), std::back_inserter(diff));
    if (diff.empty()) {
        return ProjectErrorType::TRANSFER_PROJECT;
    }

    bool isConflict = !infos.empty() && (infos[0].importType == "drag" ||
            isFileConflict(projectTypeEnum, static_cast<ProjectTypeEnum>(infos[0].projectType)));
    if (isConflict) {
        return ProjectErrorType::PROJECT_NAME_CONFLICT;
    }
    return ProjectErrorType::NO_ERRORS;
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
    systemMemoryDbPath = path;
#else
    systemMemoryDbPath = path;
#endif
}

bool ProjectExplorerManager::IsClusterData(const std::string &projectName)
{
    std::vector<Global::ProjectExplorerInfo> projectExplorerInfo =
            QueryProjectExplorer(projectName, std::vector<std::string>());
    if (projectExplorerInfo.empty()) {
        return false;
    }
    auto projectTypeEnum = static_cast<ProjectTypeEnum>(projectExplorerInfo[0].projectType);
    if (projectTypeEnum == ProjectTypeEnum::TEXT_CLUSTER || projectTypeEnum == ProjectTypeEnum::DB_CLUSTER) {
        return true;
    }
    return false;
}

bool ProjectExplorerManager::ClearProjectExplorer(const std::vector<std::string> &projectNameList)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    db->StartTransaction();
    // 根据项目名获取对应的id,如果projectNameList为空，代表要清空所有内容，因此这里多个if条件可以减少一次sql查询
    std::vector<int64_t> projectIdList;
    if (!projectNameList.empty()) {
        std::vector<ProjectExplorerInfo> projectInfos = db->QueryProjectExplorerData(projectNameList,
                                                                                     std::vector<std::string>{});
        for (const auto &item: projectInfos) {
            projectIdList.push_back(item.id);
        }
    }

    // 删除两个表中对应的数据
    if (db->DeleteFileMenu(projectNameList, std::vector<std::string>{}) &&
        db->DeleteParsedFile(projectIdList, std::vector<int64_t>{})) {
        // 删除成功，提交事务
        db->EndTransaction();
        Server::ServerLog::Info("Success to clear project explorer.");
        return true;
    }
    // 如果出现其中一个表删除失败的清空，则进行事务回滚
    db->RollbackTransaction();
    Server::ServerLog::Error("Fail to clear project explorer.");
    return false;
}

ProjectTypeEnum ProjectExplorerManager::GetProjectType(const std::vector<ProjectExplorerInfo> &projectInfo)
{
    // 外层保证入参不为空
    std::set<ProjectTypeEnum> projectTypeSet;
    for (const auto &item: projectInfo) {
        projectTypeSet.insert(static_cast<ProjectTypeEnum>(item.projectType));
    }

    if (projectTypeSet.size() == 1) {
        return *projectTypeSet.begin();
    }

    std::set<ProjectTypeEnum> dbProjectTypeSet = {ProjectTypeEnum::DB_CLUSTER, ProjectTypeEnum::DB};
    std::set<ProjectTypeEnum> dbTypeDifference;
    std::set_difference(dbProjectTypeSet.begin(), dbProjectTypeSet.end(), projectTypeSet.begin(),
                        projectTypeSet.end(), std::inserter(dbTypeDifference, dbTypeDifference.begin()));
    if (dbTypeDifference.empty()) {
        return ProjectTypeEnum::DB_CLUSTER;
    }

    std::set<ProjectTypeEnum> textProjectTypeSet = {ProjectTypeEnum::TEXT_CLUSTER, ProjectTypeEnum::TRACE};
    std::set<ProjectTypeEnum> textTypeDifference;
    std::set_difference(textProjectTypeSet.begin(), textProjectTypeSet.end(), projectTypeSet.begin(),
                        projectTypeSet.end(), std::inserter(textTypeDifference, textTypeDifference.begin()));
    if (dbTypeDifference.empty()) {
        return ProjectTypeEnum::TEXT_CLUSTER;
    }
    return *projectTypeSet.begin();
}

std::string ProjectExplorerManager::GetClusterFilePath(const std::vector<ProjectExplorerInfo> &projectInfo)
{
    for (const auto &item: projectInfo) {
        auto projectTypeEnum = static_cast<ProjectTypeEnum>(item.projectType);
        if (projectTypeEnum == ProjectTypeEnum::TEXT_CLUSTER || projectTypeEnum == ProjectTypeEnum::DB_CLUSTER) {
            return item.fileName;
        }
    }
    return "";
}

}
}
}