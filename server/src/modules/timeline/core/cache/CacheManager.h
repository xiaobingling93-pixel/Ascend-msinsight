/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef PROFILER_SERVER_CACHEMANAGER_H
#define PROFILER_SERVER_CACHEMANAGER_H
#include "SimulationSliceCacheManager.h"
#include "SliceCacheManager.h"
#include "SliceDepthCacheManager.h"
#include "SpinLockGuard.h"
#include "DomainObject.h"
namespace Dic::Module::Timeline {
class CacheManager {
public:
    static CacheManager &Instance()
    {
        static CacheManager cacheManager;
        return cacheManager;
    }
    CacheManager(const CacheManager &) = delete;
    CacheManager &operator = (const CacheManager &) = delete;
    CacheManager(CacheManager &&) = delete;
    CacheManager &operator = (CacheManager &&) = delete;
    /**
     * 清理所有缓存
     */
    void ClearAll()
    {
        simulationSliceCacheManager.ClearAll();
        sliceCacheManager.Clear();
        sliceDepthCacheManager.ClearAllCache();
    }

    /**
     * 清理单卡缓存
     * @param fileId
     */
    void ClearCacheByFileId(const std::string &fileId)
    {
        simulationSliceCacheManager.ClearCacheByFileId(fileId);
        sliceDepthCacheManager.ClearCacheByFileId(fileId);
    }

private:
    CacheManager() = default;
    ~CacheManager() = default;
    SimulationSliceCacheManager &simulationSliceCacheManager = SimulationSliceCacheManager::Instance();
    SliceCacheManager &sliceCacheManager = SliceCacheManager::Instance();
    SliceDepthCacheManager &sliceDepthCacheManager = SliceDepthCacheManager::Instance();
    SpinLock mutex;
};
}
#endif // PROFILER_SERVER_CACHEMANAGER_H