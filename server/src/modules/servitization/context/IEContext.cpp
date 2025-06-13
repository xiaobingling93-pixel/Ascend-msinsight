/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "FileUtil.h"
#include "IEContext.h"
namespace Dic::Module::IE {
IEContext& IEContext::Instance()
{
    static IEContext ieContext;
    return ieContext;
}
void IEContext::Reset()
{
    std::unique_lock<std::mutex> lock(mutex);
    databaseMap.clear();
    dbMutexMap.clear();
    folderFileIdMap.clear();
}

bool IEContext::InitDataBase(const std::string& fileId, const std::string& dbPath)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (databaseMap.count(fileId) == 0) {
        auto conn = std::make_unique<IEConnectionPool>(dbPath, dbMutexMap[fileId]);
        conn->SetMaxActiveCount();
        databaseMap.emplace(fileId, std::move(conn));
        return true;
    }
    ServerLog::Error("The file id has a connection. id:", fileId, ", old path:", databaseMap.at(fileId)->GetDbPath(),
                     ", new path:", dbPath);
    return false;
}

bool IEContext::ExecSql(const std::string& fileId, const std::string& sql)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (databaseMap.count(fileId) == 0) {
        ServerLog::Error("Can't find connection pool. fileId:", fileId);
        return false;
    }
    auto dataBase = databaseMap[fileId]->GetConnection();
    if (dataBase == nullptr) {
        ServerLog::Error("Failed to get connection! fileId:", fileId);
        return false;
    }
    return dataBase->ExecSql(sql);
}

std::shared_ptr<Database> IEContext::GetDatabase(const std::string& fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    auto it = databaseMap.find(fileId);
    if (it == databaseMap.end()) {
        ServerLog::Error("Can't find connection pool. fileId:", fileId);
        return nullptr;
    }
    return it->second->GetConnection();
}

std::string IEContext::ComputeFileIdByFolder(const std::string& folder)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (folderFileIdMap.count(folder)) {
        return folderFileIdMap[folder];
    }
    const std::string fileName = FileUtil::GetFileName(folder);
    uint64_t count = 0;
    for (const auto& item : folderFileIdMap) {
        if (item.second.find(fileName) != std::string::npos) {
            count++;
        }
    }
    std::string fileId = fileName + "_" + std::to_string(count);
    folderFileIdMap.emplace(folder, fileId);
    return fileId;
}
}  // namespace Dic::Module::IE
