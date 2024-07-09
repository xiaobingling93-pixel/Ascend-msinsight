/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
//
#ifndef PROFILER_SERVER_SLICECACHEMANAGER_H
#define PROFILER_SERVER_SLICECACHEMANAGER_H
#include <unordered_map>
#include <set>
#include <list>
#include <vector>
#include <mutex>
#include "DomainObject.h"
#include "SpinLockGuard.h"

namespace Dic::Module::Timeline {
class SliceCacheManager {
public:
    static SliceCacheManager &Instance()
    {
        static SliceCacheManager sliceCacheManager;
        return sliceCacheManager;
    }
    SliceCacheManager(const SliceCacheManager &) = delete;
    SliceCacheManager &operator = (const SliceCacheManager &) = delete;
    SliceCacheManager(SliceCacheManager &&) = delete;
    SliceCacheManager &operator = (SliceCacheManager &&) = delete;
    /* *
     * 获取对应泳道的所有算子
     * @param key
     * @return 返回的vector是先按timestamp升序，再按照id升序
     */
    std::vector<SliceDomain> GetSliceDomainVec(const std::string &trackId)
    {
        SpinLockGuard lock(mutex);
        auto it = cache.find(trackId);
        if (it == cache.end()) {
            std::vector<SliceDomain> emptyValue;
            return emptyValue;
        }
        Touch(it);
        return it->second.first;
    }

    /* *
     * 获取当前泳道所有算子深度信息
     * @param trackId
     * @param depthInfo
     * @return
     */
    bool QueryDepthInfo(const std::string &trackId, std::unordered_map<uint64_t, uint32_t> &depthInfo)
    {
        SpinLockGuard lock(mutex);
        auto it = cache.find(trackId);
        if (it == cache.end()) {
            return false;
        }
        Touch(it);
        for (const auto &item : it->second.first) {
            depthInfo[item.id] = item.depth;
        }
        return true;
    }

    /* *
     * 更新缓存，调用前需要对value按照timestamp排序，再按照id排序
     * @param trackId 对应泳道的trackId
     * @param value 对应泳道所有的简单算子信息，该vector先按照timestamp排序，再按照id排序
     */
    void UpdateSliceCache(const std::string &trackId, const std::vector<SliceDomain> &value)
    {
        SpinLockGuard lock(mutex);
        if (std::empty(value)) {
            return;
        }
        auto it = cache.find(trackId);
        if (it == cache.end()) {
            while (curCapacity >= allCapacity) {
                curCapacity -= cache[used.back()].first.size();
                cache.erase(used.back());
                used.pop_back();
            }
            used.push_front(trackId);
            cache[trackId] = {value, used.begin()};
            curCapacity += value.size();
            return;
        }
        Touch(it);
        cache[trackId] = { value, used.begin() };
    }

    std::vector<uint64_t> GetPythonFunctionIdVec(const std::string &key)
    {
        SpinLockGuard lock(mutex);
        if (pythonFilterSet.count(key) == 0) {
            std::vector<uint64_t> emptyValue;
            return emptyValue;
        }
        auto it = pythonFunctionIDCache.find(key);
        if (it == pythonFunctionIDCache.end()) {
            std::vector<uint64_t> emptyValue;
            return emptyValue;
        }
        Touch(it);
        return it->second.first;
    }

    void PutPythonFunctionIdVec(const std::string &key, const std::vector<uint64_t> &value)
    {
        SpinLockGuard lock(mutex);
        auto it = pythonFunctionIDCache.find(key);
        if (it != pythonFunctionIDCache.end()) {
            Touch(it);
        } else {
            if (pythonFunctionIDCache.size() == pythonCapacity) {
                pythonFunctionIDCache.erase(pythonFunctionIdUsed.back());
                pythonFunctionIdUsed.pop_back();
            }
            pythonFunctionIdUsed.push_front(key);
        }
        pythonFunctionIDCache[key] = { value, pythonFunctionIdUsed.begin() };
    }

    bool HavePythonFunction(const uint64_t trackId)
    {
        SpinLockGuard lock(mutex);
        if (trackIdAndPythonFunctionMap.count(trackId) > 0) {
            return trackIdAndPythonFunctionMap[trackId];
        }
        return true;
    }

    void SetPythonFunctionStatus(const uint64_t trackId, bool status)
    {
        SpinLockGuard lock(mutex);
        trackIdAndPythonFunctionMap[trackId] = status;
    }

    void UpdatePythonFilterSet(const std::string &key, bool isFilter)
    {
        SpinLockGuard lock(mutex);
        if (isFilter) {
            pythonFilterSet.emplace(key);
        } else {
            pythonFilterSet.erase(key);
        }
    }

    void Clear()
    {
        SpinLockGuard lock(mutex);
        cache.clear();
        used.clear();
        trackIdAndPythonFunctionMap.clear();
        pythonFunctionIDCache.clear();
        pythonFunctionIdUsed.clear();
        pythonFilterSet.clear();
    }

private:
    SliceCacheManager() = default;
    ~SliceCacheManager() = default;
    using VisitOrderList = std::list<std::string>;
    using CacheValue = std::pair<std::vector<SliceDomain>, VisitOrderList::iterator>;
    using PythonFunctionIDCache = std::pair<std::vector<uint64_t>, VisitOrderList::iterator>;
    using CacheMap = std::unordered_map<std::string, CacheValue>;
    using PythonFunctionMap = std::unordered_map<std::string, PythonFunctionIDCache>;

    // 算子缓存
    CacheMap cache;
    // 算子缓存使用记录
    VisitOrderList used;
    SpinLock mutex;
    // 算子缓存大小上限
    const uint64_t allCapacity = 100000000;
    // 当前缓存大小
    uint64_t curCapacity = 0;

    // trackId 是否存在python function
    std::unordered_map<uint64_t, bool> trackIdAndPythonFunctionMap;

    // 算子调用栈id缓存
    PythonFunctionMap pythonFunctionIDCache;
    // 算子调用栈id缓存使用记录
    VisitOrderList pythonFunctionIdUsed;
    // 算子调用栈id缓存大小
    int pythonCapacity = 3;
    // 过滤了python function的trackId集合
    std::set<std::string> pythonFilterSet;

    // 更新算子缓存使用记录
    void Touch(CacheMap::iterator it)
    {
        std::string key = it->first;
        used.erase(it->second.second);
        used.push_front(key);
        it->second.second = used.begin();
    }

    // 更新算子缓存使用记录
    void Touch(PythonFunctionMap::iterator it)
    {
        std::string key = it->first;
        pythonFunctionIdUsed.erase(it->second.second);
        pythonFunctionIdUsed.push_front(key);
        it->second.second = pythonFunctionIdUsed.begin();
    }
};
}
#endif // PROFILER_SERVER_SLICECACHEMANAGER_H