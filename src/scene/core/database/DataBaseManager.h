/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_DATABASEMANAGER_H
#define PROFILER_SERVER_DATABASEMANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include "TraceDatabase.h"

namespace Dic {
namespace Scene {
namespace Core {
class DataBaseManager {
public:
    static DataBaseManager &Instance();
    DataBaseManager(const DataBaseManager &) = delete;
    DataBaseManager &operator=(const DataBaseManager &) = delete;
    DataBaseManager(DataBaseManager &&) = delete;
    DataBaseManager &operator=(DataBaseManager &&) = delete;

    TraceDatabase *GetTraceDatabase(const std::string &fileId);
    void ReleaseTraceDatabase(const std::string &fileId);

private:
    DataBaseManager() = default;
    ~DataBaseManager() = default;

    std::mutex mutex;
    std::map<std::string, std::unique_ptr<TraceDatabase>> traceDatabaseMap;
};
} // end of namespace Core
} // end of namespace Scene
} // end of namespace Dic

#endif // PROFILER_SERVER_DATABASEMANAGER_H
