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


namespace Dic {
namespace Module {
namespace Timeline {
class CacheManager {
public:
    static CacheManager &Instance();
    CacheManager(const CacheManager &) = delete;
    CacheManager &operator = (const CacheManager &) = delete;
    CacheManager(CacheManager &&) = delete;
    CacheManager &operator = (CacheManager &&) = delete;
    std::vector<SliceDomain> Get(const std::string &key)
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
    }

private:
    CacheManager() = default;
    ~CacheManager() = default;
    using VisitOrderList = std::list<std::string>;
    using CacheValue = std::pair<std::vector<SliceDomain>, VisitOrderList::iterator>;
    using CacheMap = std::unordered_map<std::string, CacheValue>;

    CacheMap cache;
    VisitOrderList used;
    std::mutex mutex;
    int m_capacity = 10;

    std::unordered_map<uint64_t, bool> trackIdAndPythonFunctionMap;

    void Touch(CacheMap::iterator it)
    {
        std::string key = it->first;
        used.erase(it->second.second);
        used.push_front(key);
        it->second.second = used.begin();
    }
};
}
}
}
#endif // PROFILER_SERVER_CACHEMANAGER_H