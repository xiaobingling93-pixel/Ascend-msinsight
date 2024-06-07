/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
//
#ifndef PROFILER_SERVER_CACHEMANAGER_H
#define PROFILER_SERVER_CACHEMANAGER_H
#include <unordered_map>
#include <list>
#include <vector>
#include <mutex>
#include "DomainObject.h"

namespace Dic::Module::Timeline {
class CacheManager {
public:
    static CacheManager &Instance();
    CacheManager(const CacheManager &) = delete;
    CacheManager &operator = (const CacheManager &) = delete;
    CacheManager(CacheManager &&) = delete;
    CacheManager &operator = (CacheManager &&) = delete;
    std::vector<SliceDomain> GetSliceDomainVec(const std::string &key)
    {
        std::unique_lock<std::mutex> lock(mutex);
        auto it = cache.find(key);
        if (it == cache.end()) {
            std::vector<SliceDomain> emptyValue;
            return emptyValue;
        }
        Touch(it);
        return it->second.first;
    }

    void Put(const std::string &key, const std::vector<SliceDomain> &value)
    {
        std::unique_lock<std::mutex> lock(mutex);
        auto it = cache.find(key);
        if (it != cache.end()) {
            Touch(it);
        } else {
            if (cache.size() == m_capacity) {
                cache.erase(used.back());
                used.pop_back();
            }
            used.push_front(key);
        }
        cache[key] = { value, used.begin() };
    }

    std::vector<uint64_t> GetPythonFunctionIdVec(const std::string &key)
    {
        std::unique_lock<std::mutex> lock(mutex);
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
        std::unique_lock<std::mutex> lock(mutex);
        auto it = pythonFunctionIDCache.find(key);
        if (it != pythonFunctionIDCache.end()) {
            Touch(it);
        } else {
            if (pythonFunctionIDCache.size() == m_python_capacity) {
                pythonFunctionIDCache.erase(pythonFunctionIdUsed.back());
                pythonFunctionIdUsed.pop_back();
            }
            pythonFunctionIdUsed.push_front(key);
        }
        pythonFunctionIDCache[key] = { value, pythonFunctionIdUsed.begin() };
    }

    bool HavePythonFunction(const uint64_t trackId)
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (trackIdAndPythonFunctionMap.count(trackId) > 0) {
            return trackIdAndPythonFunctionMap[trackId];
        }
        return true;
    }

    void SetPythonFunctionStatus(const uint64_t trackId, bool status)
    {
        std::unique_lock<std::mutex> lock(mutex);
        trackIdAndPythonFunctionMap[trackId] = status;
    }

    void Clear()
    {
        std::unique_lock<std::mutex> lock(mutex);
        cache.clear();
        used.clear();
        trackIdAndPythonFunctionMap.clear();
        pythonFunctionIDCache.clear();
        pythonFunctionIdUsed.clear();
    }

private:
    CacheManager() = default;
    ~CacheManager() = default;
    using VisitOrderList = std::list<std::string>;
    using CacheValue = std::pair<std::vector<SliceDomain>, VisitOrderList::iterator>;
    using PythonFunctionIDCache = std::pair<std::vector<uint64_t >, VisitOrderList::iterator>;
    using CacheMap = std::unordered_map<std::string, CacheValue>;
    using PythonFunctionMap = std::unordered_map<std::string, PythonFunctionIDCache>;

    // 算子缓存
    CacheMap cache;
    // 算子缓存使用记录
    VisitOrderList used;
    std::mutex mutex;
    // 算子缓存大小
    int m_capacity = 10;

    // trackId 是否存在python function
    std::unordered_map<uint64_t, bool> trackIdAndPythonFunctionMap;

    // 算子调用栈id缓存
    PythonFunctionMap pythonFunctionIDCache;
    // 算子调用栈id缓存使用记录
    VisitOrderList pythonFunctionIdUsed;
    // 算子调用栈id缓存大小
    int m_python_capacity = 3;

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
#endif // PROFILER_SERVER_CACHEMANAGER_H