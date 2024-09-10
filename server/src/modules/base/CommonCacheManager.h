/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
//
#ifndef PROFILER_SERVER_COMMON_CACHE_MANAGER_H
#define PROFILER_SERVER_COMMON_CACHE_MANAGER_H
#include <unordered_map>
#include <list>
#include <vector>
#include <mutex>
#include "algorithm"
#include "StringUtil.h"
#include "TimelineProtocolResponse.h"


namespace Dic::Module {
using namespace Protocol;

class CommonCacheManager {
public:
    static CommonCacheManager &Instance()
    {
        static CommonCacheManager cacheManager;
        return cacheManager;
    }
    CommonCacheManager(const CommonCacheManager &) = delete;
    CommonCacheManager &operator = (const CommonCacheManager &) = delete;
    CommonCacheManager(CommonCacheManager &&) = delete;
    CommonCacheManager &operator = (CommonCacheManager &&) = delete;

    const std::vector<UnitSingleFlow> GetFlowCache(const std::string &rankId, const std::string &cat)
    {
        return flowCache[rankId][cat];
    }

    void Put(const std::string &rankId, const std::string &cat, UnitSingleFlow flow)
    {
        std::lock_guard<std::mutex> lock(mutex);
        flowCache[rankId][cat].emplace_back(flow);
    }

    void SetFlowState(const std::string &rankId, bool state)
    {
        std::lock_guard<std::mutex> lock(mutex);
        flowCacheState[rankId] = state;
        flowCv.notify_all();
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(mutex);
        flowCache.clear();
        flowCacheState.clear();
    }

    void EraseFlowByRank(const std::string &rankId)
    {
        std::lock_guard<std::mutex> lock(mutex);
        auto iter = flowCache.find(rankId);
        if (iter != flowCache.end()) {
            flowCache.erase(iter);
        }
        flowCacheState[rankId] = false;
        flowCv.notify_all();
    }

private:
    CommonCacheManager() = default;
    ~CommonCacheManager() = default;

    using FlowCacheMap = std::unordered_map<std::string, std::unordered_map<std::string, std::vector<UnitSingleFlow>>>;

    std::mutex mutex;
    FlowCacheMap flowCache;
    std::unordered_map<std::string, bool> flowCacheState;
    std::condition_variable flowCv;
};
}
#endif // PROFILER_SERVER_COMMON_CACHE_MANAGER_H