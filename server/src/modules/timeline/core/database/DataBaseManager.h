/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_DATABASEMANAGER_H
#define PROFILER_SERVER_DATABASEMANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include "ConnectionPool.h"
#include "TraceDatabase.h"
#include "../../../base/core/ClusterDatabase.h"
#include "MemoryDataBase.h"
#include "KernelParse.h"

namespace Dic {
namespace Module {
namespace Timeline {
enum class DatabaseType {
    TRACE,
    SUMMARY,
    MEMORY
};
class DataBaseManager {
public:
    static DataBaseManager &Instance();
    DataBaseManager(const DataBaseManager &) = delete;
    DataBaseManager &operator=(const DataBaseManager &) = delete;
    DataBaseManager(DataBaseManager &&) = delete;
    DataBaseManager &operator=(DataBaseManager &&) = delete;

    Memory::MemoryDataBase *GetMemoryDatabase(const std::string &fileId);
    Summary::SummaryDataBase *GetSummaryDatabase(const std::string &fileId);
    bool CreatConnectionPool(const std::string &fileId, const std::string &dbPath);
    std::shared_ptr<TraceDatabase> GetTraceDatabase(const std::string &fileId);
    std::vector<ConnectionPool *> GetAllTraceDatabase();
    std::vector<std::string> GetAllFileId();
    void Clear();
    void Clear(DatabaseType type);
    void ClearClusterDb();
    void ReleaseTraceDatabase(const std::string &fileId);
    bool HasFileId(DatabaseType type, const std::string &fileId);
    ClusterDatabase *GetClusterDatabase();

    std::vector<Memory::MemoryDataBase *> GetAllMemoryDatabase();

    std::vector<Summary::SummaryDataBase *> GetAllSummaryDatabase();
    std::string GetDbPath(const std::string &fileId);

private:
    DataBaseManager() = default;
    ~DataBaseManager() = default;

    std::mutex mutex;
    std::map<std::string, std::unique_lock<std::mutex>> dbMutexMap;
    std::map<std::string, std::unique_ptr<ConnectionPool>> traceDatabaseMap;
    std::map<std::string, std::unique_ptr<ClusterDatabase>> clusterDatabaseMap;
    std::map<std::string, std::unique_ptr<Memory::MemoryDataBase>> memoryDatabaseMap;
    std::map<std::string, std::unique_ptr<Summary::SummaryDataBase>> summaryDatabaseMap;

    std::mutex &GetDbMutex(const std::string &fileId);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_DATABASEMANAGER_H
