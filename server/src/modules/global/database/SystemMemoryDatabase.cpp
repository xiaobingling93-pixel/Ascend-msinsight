/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "GlobalDefs.h"
#include "SystemMemoryDatabase.h"

namespace Dic {
namespace Module {
namespace Global {
using namespace Server;
bool SystemMemoryDatabase::SetConfig()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    std::string dbVersion = GetDataBaseVersion();
    std::unique_lock<std::recursive_mutex> lock(mutex);
    return ExecSql("PRAGMA synchronous = OFF; PRAGMA journal_mode = MEMORY; PRAGMA user_version = " + dbVersion + ";");
}

bool SystemMemoryDatabase::CreateTable()
{
    if (!isOpen) {
        ServerLog::Error("Failed to create table. Database is not open.");
        return false;
    }
    if (CheckTableExist(projectExplorerTable)) {
        return true;
    }
    std::string sql = "CREATE TABLE " + projectExplorerTable + " ( id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                                               "projectName TEXT, fileName TEXT, projectType INTEGER, "
                                                               "importType TEXT, dbPath Text );";
    std::unique_lock<std::recursive_mutex> lock(mutex);
    return ExecSql(sql);
}

bool SystemMemoryDatabase::DropTable()
{
    std::vector<std::string> tables = {projectExplorerTable};
    std::unique_lock<std::recursive_mutex> lock(mutex);
    return DropSomeTables(tables);
}

bool SystemMemoryDatabase::SaveProjectExplorerData(ProjectExplorerInfo projectExplorerInfo)
{
    if (projectExplorerInfo.projectName.empty() || projectExplorerInfo.fileName.empty()) {
        ServerLog::Error("Failed to save FileMenuData, params is invalid.");
        return false;
    }
    std::unique_lock<std::recursive_mutex> lock(mutex);
    std::string sql = "INSERT INTO " + projectExplorerTable + "(projectName, fileName, projectType, importType, dbPath)"
                                                              " VALUES(?, ?, ?, ?, ?);";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to save FileMenuData, prepared statement failed: projectName=",
                         projectExplorerInfo.projectName, ", fileName=", projectExplorerInfo.fileName);
        return false;
    }
    std::string dbPath = StringUtil::join(projectExplorerInfo.dbPath, ",");
    stmt->BindParams(projectExplorerInfo.projectName, projectExplorerInfo.fileName,
                     projectExplorerInfo.projectType, projectExplorerInfo.importType, dbPath);
    if (!stmt->Execute()) {
        ServerLog::Error("Failed to save FileMenuData, stmt execute failed: projectName=",
                         projectExplorerInfo.projectName, ", fileName=", projectExplorerInfo.fileName);
        return false;
    }
    return true;
}

bool SystemMemoryDatabase::UpdateProjectName(const std::string &oldProjectName, const std::string &newProjectName)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (oldProjectName.empty() || newProjectName.empty()) {
        ServerLog::Error("Failed to update project name, param is invalid.");
        return false;
    }
    if (!CheckTableExist(projectExplorerTable)) {
        ServerLog::Error("Failed to update project name, table is not exist.");
        return false;
    }
    std::string sql = "Update " + projectExplorerTable + " SET projectName = ? WHERE projectName = ?;";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to create prepared stmt: oldProjectName=", oldProjectName,
                         ", newProjectName=", newProjectName);
        return false;
    }
    stmt->BindParams(newProjectName, oldProjectName);
    if (!stmt->Execute()) {
        ServerLog::Error("Failed to update project name: oldProjectName=", oldProjectName,
                         ", newProjectName=", newProjectName);
        return false;
    }
    return true;
}

std::vector<ProjectExplorerInfo> SystemMemoryDatabase::QueryProjectExplorerData(
    const std::string &projectName, const std::vector<std::string>& fileNameList)
{
    std::vector<ProjectExplorerInfo> res;
    std::string sql = "SELECT projectName, fileName, projectType, importType, dbPath FROM " + projectExplorerTable +
            " WHERE 1 = 1";
    if (!projectName.empty()) {
        sql += " and projectName = ?";
    }
    if (!fileNameList.empty()) {
        sql += " and fileName in (?)";
    }
    sql += ";";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to query FileMenuData ! Failed to prepare sql.", sqlite3_errmsg(db));
        return res;
    }
    int index = bindStartIndex;
    if (!projectName.empty()) {
        sqlite3_bind_text(stmt, index++, projectName.c_str(), projectName.length(), nullptr);
    }
    if (!fileNameList.empty()) {
        std::string fileNameStr = StringUtil::join(fileNameList, ", ");
        sqlite3_bind_text(stmt, index, fileNameStr.c_str(), fileNameStr.length(), nullptr);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        ProjectExplorerInfo info{};
        info.projectName = sqlite3_column_string(stmt, col++);
        info.fileName = sqlite3_column_string(stmt, col++);
        info.projectType = sqlite3_column_int64(stmt, col++);
        info.importType = sqlite3_column_string(stmt, col++);
        info.dbPath = StringUtil::Split(sqlite3_column_string(stmt, col++), ",");
        res.emplace_back(info);
    }
    sqlite3_finalize(stmt);
    return res;
}

bool SystemMemoryDatabase::DeleteFileMenu(const std::string &projectName, const std::vector<std::string>& fileNameList)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (projectName.empty()) {
        ServerLog::Error("Failed to delete by project name, param is invalid.");
        return false;
    }
    if (!CheckTableExist(projectExplorerTable)) {
        ServerLog::Error("Failed to delete by project name, table is not exist.");
        return false;
    }
    std::string sql = "DELETE FROM " + projectExplorerTable + " WHERE projectName = ?";
    if (!fileNameList.empty()) {
        sql += " and fileName in (?)";
    }
    sql += ";";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to delete by project name, failed to create prepared stmt: projectName=", projectName);
        return false;
    }
    if (fileNameList.empty()) {
        stmt->BindParams(projectName);
    } else {
        std::string fileNameStr = StringUtil::join(fileNameList, ", ");
        stmt->BindParams(projectName, fileNameStr);
    }
    if (!stmt->Execute()) {
        ServerLog::Error("Failed to delete info by project name: projectName=", projectName);
        return false;
    }
    return true;
}

bool SystemMemoryDatabase::UpdateProjectDbPath(const std::string &projectName, const std::string &fileName,
                                               const std::string &dbPath)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (projectName.empty() || fileName.empty()) {
        ServerLog::Error("Failed to update ProjectDbPath, param is invalid.");
        return false;
    }
    if (!CheckTableExist(projectExplorerTable)) {
        ServerLog::Error("Failed to update ProjectName failed, table is not exist.");
        return false;
    }
    std::string sql = "Update " + projectExplorerTable + " SET dbPath = ? WHERE projectName = ? AND fileName = ?;";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to create prepared stmt: projectName=", projectName, ", fileName=", fileName);
        return false;
    }
    stmt->BindParams(dbPath, projectName, fileName);
    if (!stmt->Execute()) {
        ServerLog::Error("Failed to update project db path: projectName=", projectName,
                         ", fileName=", fileName);
        return false;
    }
    return true;
}
}
}
}