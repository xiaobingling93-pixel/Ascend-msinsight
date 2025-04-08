/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_DATABASEMANAGER_H
#define PROFILER_SERVER_DATABASEMANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <unordered_set>
#include "ConnectionPool.h"
#include "TextTraceDatabase.h"
#include "DbTraceDataBase.h"
#include "TextClusterDatabase.h"
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
    TEXT,
    DB
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
    void EraseClusterDb(const std::string &uniqueKey);
    void ClearClusterDb();
    void ReleaseDatabase(const std::string &fileId);
    bool HasFileId(DatabaseType type, const std::string &fileId);
    std::shared_ptr<VirtualClusterDatabase> CreateClusterDatabase(const std::string &uniqueKey, DataType type);
    std::shared_ptr<VirtualClusterDatabase> GetClusterDatabase(const std::string &uniqueKey);

    std::shared_ptr<Memory::VirtualMemoryDataBase> GetMemoryDatabase(const std::string &fileId);
    std::vector<Memory::VirtualMemoryDataBase *> GetAllMemoryDatabase();

    std::shared_ptr<Summary::VirtualSummaryDataBase> GetSummaryDatabase(const std::string &fileId);
    std::vector<Summary::VirtualSummaryDataBase *> GetAllSummaryDatabase();

    std::string GetDbPath(const std::string &fileId);
    std::shared_ptr<VirtualTraceDatabase> GetTraceDatabaseWithOutHost(const std::string &rankId);
    DataType GetDataType();
    void SetDataType(DataType type);
    FileType GetFileType();
    FileType GetFileTypeByRankId(const std::string &rankId);
    void SetFileType(FileType type);
    void SetBaselineDataType(DataType type);
    void SetBaselineFileType(FileType type);
    bool ResetBaseline();
    void SetDbPathMapping(const std::string& fileId, const std::string& filePath, const std::string& hostId);
    bool IsContainDatabasePath(const std::string& databasePath);
    inline std::vector<std::string> GetDbPathByHost(const std::string& id)
    {
        if (host2DbPath.find(id) != host2DbPath.end()) {
            return host2DbPath[id];
        }
        return {};
    }

private:
    DataBaseManager() = default;
    ~DataBaseManager() = default;

    std::recursive_mutex mutex;
    DataType dataType = DataType::TEXT;
    DataType baselineType = DataType::TEXT;
    FileType fileType = FileType::PYTORCH;
    std::map<std::string, std::recursive_mutex> dbMutexMap;
    std::map<std::string, std::string> dbFilePathMap;
    std::map<std::string, std::vector<std::string>> host2DbPath;
    std::unordered_set<std::string> databasePathSet;
    std::map<std::string, std::unique_ptr<ConnectionPool>> traceDatabaseMap;
    std::map<std::string, std::shared_ptr<VirtualClusterDatabase>> clusterDatabaseMap;
    std::map<std::string, std::shared_ptr<Memory::VirtualMemoryDataBase>> memoryDatabaseMap;
    std::map<std::string, std::shared_ptr<Summary::VirtualSummaryDataBase>> summaryDatabaseMap;

    FileType baselineFileType = FileType::PYTORCH;
    std::map<std::string, std::unique_ptr<ConnectionPool>> traceBaselineDatabaseMap;
    std::map<std::string, std::shared_ptr<Memory::VirtualMemoryDataBase>> memoryBaselineDatabaseMap;
    std::map<std::string, std::shared_ptr<Summary::VirtualSummaryDataBase>> summaryBaselineDatabaseMap;

    std::recursive_mutex &GetDbMutex(const std::string &fileId);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_DATABASEMANAGER_H
