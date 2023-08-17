/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "DataBaseManager.h"
namespace Dic {
namespace Scene {
namespace Core {

DataBaseManager &DataBaseManager::Instance()
{
    static DataBaseManager instance;
    return instance;
}

TraceDatabase *DataBaseManager::GetTraceDatabase(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (traceDatabaseMap.count(fileId) == 0) {
        traceDatabaseMap.emplace(fileId, std::make_unique<TraceDatabase>());
    }
    return traceDatabaseMap[fileId].get();
}

void DataBaseManager::ReleaseTraceDatabase(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (traceDatabaseMap.count(fileId) >= 0) {
        traceDatabaseMap.erase(fileId);
    }
}

bool DataBaseManager::HasFileId(const std::string &fileId)
{
    std::unique_lock<std::mutex> lock(mutex);
    return traceDatabaseMap.count(fileId) != 0;
}
} // end of namespace Core
} // end of namespace Scene
} // end of namespace Dic
