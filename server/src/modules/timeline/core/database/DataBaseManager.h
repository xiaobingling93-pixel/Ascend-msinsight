/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_DATABASEMANAGER_H
#define PROFILER_SERVER_DATABASEMANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include "ConnectionPool.h"
#include "JsonTraceDatabase.h"
#include "DbTraceDataBase.h"
#include "JsonClusterDatabase.h"
#include "VirtualMemoryDataBase.h"
#include "KernelParse.h"

namespace Dic {
namespace Module {
namespace Timeline {
enum class DatabaseType {
    TRACE,
    SUMMARY,
    MEMORY
};
enum class DataType {
    JSON,
    FULL_DB
};
enum class FileType {
    MS_PROF,
    PYTORCH
};
class DataBaseManager {
public:
    static DataBaseManager &Instance();
    DataBaseManager(const DataBaseManager &) = delete;
    DataBaseManager &operator=(const DataBaseManager &) = delete;
    DataBaseManager(DataBaseManager &&) = delete;
    DataBaseManager &operator=(DataBaseManager &&) = delete;

    bool CreatConnectionPool(const std::string &fileId, const std::string &dbPath);
    std::shared_ptr<VirtualTraceDatabase> GetTraceDatabase(const std::string &fileId);
    std::vector<ConnectionPool *> GetAllTraceDatabase();
    std::vector<std::string> GetAllFileId();
    void Clear();
    void Clear(DatabaseType type);
    void ClearClusterDb();
    void ReleaseDatabase(const std::string &fileId);
    bool HasFileId(DatabaseType type, const std::string &fileId);
    VirtualClusterDatabase *GetWriteClusterDatabase();
    VirtualClusterDatabase *GetReadClusterDatabase();

    Memory::VirtualMemoryDataBase *GetMemoryDatabase(const std::string &fileId);
    std::vector<Memory::VirtualMemoryDataBase *> GetAllMemoryDatabase();

    Summary::VirtualSummaryDataBase *GetSummaryDatabase(const std::string &fileId);
    std::vector<Summary::VirtualSummaryDataBase *> GetAllSummaryDatabase();

    std::string GetDbPath(const std::string &fileId);
    DataType GetDataType();
    void SetDataType(DataType type);
    FileType GetFileType();
    void SetFileType(FileType type);
    void SetDbPathMapping(const std::string& rankId, const std::string& filePath);

    bool curIsCluster = false;
    bool curIsDb = false;
    bool curIsBin = false;

private:
    DataBaseManager() = default;
    ~DataBaseManager() = default;

    std::mutex mutex;
    DataType dataType = DataType::JSON;
    FileType fileType = FileType::PYTORCH;
    std::map<std::string, std::recursive_mutex> dbMutexMap;
    std::map<std::string, std::string> dbFilePathMap;
    std::map<std::string, std::unique_ptr<ConnectionPool>> traceDatabaseMap;
    std::map<std::string, std::unique_ptr<VirtualClusterDatabase>> clusterDatabaseMap;
    std::map<std::string, std::unique_ptr<Memory::VirtualMemoryDataBase>> memoryDatabaseMap;
    std::map<std::string, std::unique_ptr<Summary::VirtualSummaryDataBase>> summaryDatabaseMap;

    std::recursive_mutex &GetDbMutex(const std::string &fileId);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_DATABASEMANAGER_H
