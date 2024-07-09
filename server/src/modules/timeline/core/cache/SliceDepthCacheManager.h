/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SLICEDEPTHCACHEMANAGER_H
#define PROFILER_SERVER_SLICEDEPTHCACHEMANAGER_H
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <vector>
#include <mutex>
#include "SpinLockGuard.h"
#include "Timer.h"
namespace Dic::Module::Timeline {
struct SliceDepthCacheStruct {
    uint64_t trackId = 0;
    int32_t maxDepTh = 0;
    std::unordered_map<uint64_t, int32_t> sliceIdAndDepthMap;
};
class SliceDepthCacheManager {
public:
    static SliceDepthCacheManager &Instance()
    {
        static SliceDepthCacheManager sliceDepthCacheManager;
        return sliceDepthCacheManager;
    }
    SliceDepthCacheManager(const SliceDepthCacheManager &) = delete;
    SliceDepthCacheManager &operator = (const SliceDepthCacheManager &) = delete;
    SliceDepthCacheManager(SliceDepthCacheManager &&) = delete;
    SliceDepthCacheManager &operator = (SliceDepthCacheManager &&) = delete;
    SliceDepthCacheStruct GetSliceDepthCacheStructByTrackId(const uint64_t trackId)
    {
        SpinLockGuard lock(mutex);
        SliceDepthCacheStruct sliceDepthCacheStruct = depthCache[trackId];
        return sliceDepthCacheStruct;
    }

    void PutSliceDepthCacheStructByFileIdAndTrackId(const uint64_t trackId,
                                                    SliceDepthCacheStruct &sliceDepthCacheStruct)
    {
        depthCache[trackId] = std::move(sliceDepthCacheStruct);
    }

    int32_t QueryMaxDepthByTrackId(const uint64_t trackId)
    {
        int32_t depth = depthCache[trackId].maxDepTh;
        return depth;
    }

    void ClearAllCache()
    {
        SpinLockGuard lock(mutex);
        fileIdAndTrackIdsCache.clear();
        depthCache.clear();
    }

    void ClearCacheByFileId(const std::string &fileId)
    {
        SpinLockGuard lock(mutex);
        std::unordered_set<uint64_t> trackIds = fileIdAndTrackIdsCache[fileId];
        for (const auto &item : trackIds) {
            depthCache.erase(item);
        }
        fileIdAndTrackIdsCache.erase(fileId);
    }

private:
    SliceDepthCacheManager() = default;
    ~SliceDepthCacheManager() = default;

    using SlingTrackDepthMap = std::unordered_map<uint64_t, SliceDepthCacheStruct>;

    SlingTrackDepthMap depthCache;
    std::unordered_map<std::string, std::unordered_set<uint64_t>> fileIdAndTrackIdsCache;
    SpinLock mutex;
};
}


#endif // PROFILER_SERVER_SLICEDEPTHCACHEMANAGER_H
