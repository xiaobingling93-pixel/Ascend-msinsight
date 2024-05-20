/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <SliceAnalyzer.h>
namespace Dic::Module::Timeline {
SliceAnalyzer::SliceAnalyzer() = default;
/**
 * 对前端要展示的算子进行取样
 * @param startTime 页面开始时间
 * @param endTime 页面结束时间
 * @param minTimestamp 最小时间
 * @param cacheSlices 当前泳道内的所有算子数据，按照depth排序，depth相同则按照timestamp排序
 * @return
 */
std::set<int64_t> SliceAnalyzer::ComputeResultIds(uint64_t startTime, uint64_t endTime, uint64_t minTimestamp,
    std::vector<CacheSlice> &cacheSlices)
{
    // 根据开始时间结束时间把屏幕平均分成1000份
    const int maxDataCount = 1000;
    uint64_t unitTime = (endTime - startTime) / maxDataCount;
    unitTime = unitTime <= 0 ? 1 : unitTime;
    std::set<int64_t> ids;
    int32_t curDepth = -1;
    // 第一份屏幕的截至时间
    uint64_t curLimitTime = startTime + unitTime + minTimestamp;
    int64_t tempId = 0;
    uint64_t tempDuration = 0;
    for (const auto &item : cacheSlices) {
        // 算子深度大于当前深度，则把tempId加进结果集，重置tempId
        if (item.depth > curDepth) {
            ids.emplace(tempId);
            tempId = 0;
            tempDuration = 0;
            curLimitTime = startTime + unitTime + minTimestamp;
            curDepth = item.depth;
        }
        // 算子结束时间小于屏幕最小时间，过滤
        if (item.timestamp + item.duration < startTime + minTimestamp) {
            continue;
        }
        // 算子开始时间大于屏幕最大时间，过滤
        if (item.timestamp > endTime + minTimestamp) {
            continue;
        }
        // 算子开始时间大于当前份屏幕时间，则把tempId加进结果集，重置tempId，进入下一份屏幕采样
        while (item.timestamp > curLimitTime && curLimitTime <= endTime + minTimestamp) {
            ids.emplace(tempId);
            tempId = 0;
            tempDuration = 0;
            curLimitTime += unitTime;
        }
        // 更新tempId
        if (tempDuration <= item.duration) {
            tempId = item.id;
            tempDuration = item.duration;
        }
        // 如果算子很长，则找下一份屏幕的截至时间
        while (item.timestamp + item.duration >= curLimitTime && curLimitTime <= endTime + minTimestamp) {
            ids.emplace(tempId);
            tempId = 0;
            tempDuration = 0;
            curLimitTime += unitTime;
        }
    }
    ids.emplace(tempId);
    return ids;
}

void SliceAnalyzer::SortByTimestampASC(std::vector<CacheSlice> &cacheSlices)
{
    std::sort(cacheSlices.begin(), cacheSlices.end(), SliceAnalyzer::CompareTimestampASC);
}

uint32_t SliceAnalyzer::ComputeFlowPointDepth(std::vector<CacheSlice> &cacheSlices, std::string &type,
    uint64_t timestamp)
{
    CacheSlice cacheSlice;
    cacheSlice.timestamp = timestamp;
    cacheSlice.id = 0;
    if (type == Protocol::LINE_START) {
        auto it = std::lower_bound(cacheSlices.begin(), cacheSlices.end(), cacheSlice, CompareTimestampASC);
        if (it != cacheSlices.end() && it->timestamp == timestamp) {
            return it->depth;
        }

        if (it != cacheSlices.end() && it > cacheSlices.begin()) {
            return (it--)->depth;
        }
    }
    if (type == Protocol::LINE_END || type == Protocol::LINE_END_OPTIONAL) {
        auto it = std::lower_bound(cacheSlices.begin(), cacheSlices.end(), cacheSlice, CompareTimestampASC);
        if (it != cacheSlices.end()) {
            return it->depth;
        }
    }
    return 0;
}

bool SliceAnalyzer::CompareTimestampASC(const CacheSlice &first, const CacheSlice &second)
{
    if (first.timestamp < second.timestamp) {
        return true;
    }
    if (first.timestamp == second.timestamp && first.id < second.id) {
        return true;
    }
    return false;
}

}
