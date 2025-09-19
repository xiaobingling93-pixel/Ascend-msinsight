/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_SYSTEMVIEWOVERALLCACHEMANAGER_H
#define PROFILER_SERVER_SYSTEMVIEWOVERALLCACHEMANAGER_H
#include <map>
#include <vector>
#include <string>
#include <shared_mutex>
#include "TimelineProtocolResponse.h"

namespace Dic::Module::Timeline {
class SystemViewOverallCacheManager {
public:
    static SystemViewOverallCacheManager &Instance()
    {
        static SystemViewOverallCacheManager overallCacheManager;
        return overallCacheManager;
    }
    SystemViewOverallCacheManager(const SystemViewOverallCacheManager &) = delete;
    SystemViewOverallCacheManager &operator = (const SystemViewOverallCacheManager &) = delete;
    SystemViewOverallCacheManager(SystemViewOverallCacheManager &&) = delete;
    SystemViewOverallCacheManager &operator = (SystemViewOverallCacheManager &&) = delete;

    void ClearAll()
    {
        std::unique_lock<std::shared_mutex> uniqueLock(sharedMutex);
        systemViewOverallCacheMap.clear();
    }

    void SetOverallData(const std::string &key, const std::vector<Dic::Protocol::SystemViewOverallRes> &overallList)
    {
        std::unique_lock<std::shared_mutex> uniqueLock(sharedMutex);
        systemViewOverallCacheMap[key] = overallList;
    }

    std::vector<Dic::Protocol::SystemViewOverallRes> GetOverallData(const std::string &key)
    {
        std::unique_lock<std::shared_mutex> uniqueLock(sharedMutex);
        auto it = systemViewOverallCacheMap.find(key);
        if (it == systemViewOverallCacheMap.end()) {
            return {};
        }
        return it->second;
    }

private:
    SystemViewOverallCacheManager() = default;
    ~SystemViewOverallCacheManager() = default;
    std::map<std::string, std::vector<Dic::Protocol::SystemViewOverallRes>> systemViewOverallCacheMap;
    std::shared_mutex sharedMutex;
};
}
#endif // PROFILER_SERVER_SYSTEMVIEWOVERALLCACHEMANAGER_H
