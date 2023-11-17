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

namespace Dic {
namespace Module {
namespace Timeline {
class DataBaseManager {
public:
    static DataBaseManager &Instance();
    DataBaseManager(const DataBaseManager &) = delete;
    DataBaseManager &operator=(const DataBaseManager &) = delete;
    DataBaseManager(DataBaseManager &&) = delete;
    DataBaseManager &operator=(DataBaseManager &&) = delete;

    bool CreatConnectionPool(const std::string &fileId, const std::string &dbPath);
    std::shared_ptr<TraceDatabase> GetTraceDatabase(const std::string &fileId);
    std::vector<ConnectionPool *> GetAllTraceDatabase();
    std::vector<std::string> GetAllFileId();
    void Clear();
    void ReleaseTraceDatabase(const std::string &fileId);
    bool HasFileId(const std::string &fileId);
    std::string GetDbPath(const std::string &fileId);

private:
    DataBaseManager() = default;
    ~DataBaseManager() = default;

    std::mutex mutex;
    std::map<std::string, std::unique_ptr<ConnectionPool>> traceDatabaseMap;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_DATABASEMANAGER_H
