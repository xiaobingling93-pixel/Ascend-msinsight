/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_DATABASEMANAGER_H
#define PROFILER_SERVER_DATABASEMANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include "TraceDatabase.h"
#include "MemoryDataBase.h"

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

    TraceDatabase *GetTraceDatabase(const std::string &fileId);
    Memory::MemoryDataBase *GetMemoryDatabase(const std::string &fileId);
    std::vector<TraceDatabase *> GetAllTraceDatabase();
    void Clear();
    void ReleaseTraceDatabase(const std::string &fileId);
    bool HasFileId(const std::string &fileId);

    std::vector<Memory::MemoryDataBase *> GetAllMemoryDatabase();

private:
    DataBaseManager() = default;
    ~DataBaseManager() = default;

    std::mutex mutex;
    std::map<std::string, std::unique_ptr<TraceDatabase>> traceDatabaseMap;
    std::map<std::string, std::unique_ptr<Memory::MemoryDataBase>> memoryDatabaseMap;

    bool MemoryHasFileId(const std::string &fileId);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_DATABASEMANAGER_H
