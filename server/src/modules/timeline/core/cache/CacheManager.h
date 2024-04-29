/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */
//
#ifndef PROFILER_SERVER_CACHEMANAGER_H
#define PROFILER_SERVER_CACHEMANAGER_H
#include <unordered_map>
#include <list>
#include <vector>
#include <mutex>


namespace Dic {
namespace Module {
namespace Timeline {
struct CacheSlice {
    int64_t id = 0;
    uint64_t timestamp = 0;
    uint64_t duration = 0;
    uint64_t endTime = 0;
    int32_t depth = 0;
    bool operator < (const CacheSlice& right) const
    {
        if (depth < right.depth) {
            return true;
        }
        if (depth == right.depth && timestamp < right.timestamp) {
            return true;
        }
        return false;
    }
};
class CacheManager {
public:
    static CacheManager &Instance();
    CacheManager(const CacheManager &) = delete;
    CacheManager &operator = (const CacheManager &) = delete;
    CacheManager(CacheManager &&) = delete;
    CacheManager &operator = (CacheManager &&) = delete;
    std::vector<CacheSlice> Get(const std::string &key)
    {
        std::unique_lock<std::mutex> lock(mutex);
        auto it = cache.find(key);
        if (it == cache.end()) {
            std::vector<CacheSlice> emptyValue;
            return emptyValue;
        }
        Touch(it);
        return it->second.first;
    }

    void Put(const std::string &key, const std::vector<CacheSlice> &value)
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

    void Clear()
    {
        std::unique_lock<std::mutex> lock(mutex);
        cache.clear();
        used.clear();
    }

private:
    CacheManager() = default;
    ~CacheManager() = default;
    using VisitOrderList = std::list<std::string>;
    using CacheValue = std::pair<std::vector<CacheSlice>, VisitOrderList::iterator>;
    using CacheMap = std::unordered_map<std::string, CacheValue>;

    CacheMap cache;
    VisitOrderList used;
    std::mutex mutex;
    int m_capacity = 10;

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