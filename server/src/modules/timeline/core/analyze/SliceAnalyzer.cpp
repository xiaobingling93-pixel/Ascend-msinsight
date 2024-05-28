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
    std::set<int64_t> ids;
    if (unitTime == 0) {
        ComputeSmallScreenSliceIds(startTime, endTime, minTimestamp, cacheSlices, ids);
        return ids;
    }
    int32_t curDepth = -1;
    // 第一份屏幕的截至时间
    uint64_t curLimitTime = startTime + unitTime + minTimestamp;
    int64_t tempId = 0;
    uint64_t tempDuration = 0;
    for (const auto &item : cacheSlices) {
        // 算子结束时间小于屏幕最小时间，过滤
        if (item.timestamp + item.duration < startTime + minTimestamp) {
            continue;
        }
        // 算子开始时间大于屏幕最大时间，过滤
        if (item.timestamp > endTime + minTimestamp) {
            continue;
        }
        // 算子深度大于当前深度，则把tempId加进结果集，重置tempId
        if (item.depth > curDepth) {
            ids.emplace(tempId);
            tempId = 0;
            tempDuration = 0;
            curLimitTime = startTime + unitTime + minTimestamp;
            curDepth = item.depth;
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

/**
 * 当屏幕范围小于1000ns时的采集方式
 * @param startTime
 * @param endTime
 * @param minTimestamp
 * @param cacheSlices
 * @param ids
 */
void SliceAnalyzer::ComputeSmallScreenSliceIds(uint64_t startTime, uint64_t endTime, uint64_t minTimestamp,
    std::vector<CacheSlice> &cacheSlices, std::set<int64_t> &ids)
{
    for (const auto &item : cacheSlices) {
        // 算子结束时间小于屏幕最小时间，过滤
        if (item.timestamp + item.duration < startTime + minTimestamp) {
            continue;
        }
        // 算子开始时间大于屏幕最大时间，过滤
        if (item.timestamp > endTime + minTimestamp) {
            continue;
        }
        ids.emplace(item.id);
    }
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

/**
 * 计算每个算子自身执行时间
 * @param rows 所有算子
 * @param selfTimeKeyValue 计算结果
 * @param startTime 开始时间
 * @param endTime 结束时间
 */
void SliceAnalyzer::CalculateSelfTime(std::vector<Protocol::SimpleSlice> &rows,
    std::map<std::string, uint64_t> &selfTimeKeyValue, uint64_t startTime, uint64_t endTime)
{
    int length = rows.size();
    // offset变量用来优化性能
    int offset = 0;
    for (int i = 0; i < length; i++) {
        int32_t curDepth = rows[i].depth;
        uint64_t selfTime = rows[i].duration;
        uint64_t curSliceStartTime = rows[i].timestamp;
        uint64_t curSliceEndTime = rows[i].endTime;
        uint64_t tempStartTime = 0;
        uint64_t tempEndTime = 0;
        for (int j = offset; j < length; ++j) {
            Protocol::SimpleSlice item = rows[j];
            if (item.timestamp < curSliceStartTime) {
                continue;
            }
            if (item.depth < curDepth + 1) {
                continue;
            }
            if (item.depth > curDepth + 1) {
                break;
            }
            if (item.endTime > curSliceEndTime) {
                break;
            }
            if (item.timestamp > tempEndTime) {
                selfTime = selfTime - (tempEndTime - tempStartTime);
                tempStartTime = item.timestamp;
                tempEndTime = item.endTime;
                offset = j;
                continue;
            }
            tempEndTime = tempEndTime < item.endTime ? item.endTime : tempEndTime;
            offset = j;
        }
        selfTime = selfTime - (tempEndTime - tempStartTime);
        AddData(selfTimeKeyValue, rows[i].name, selfTime);
    }
}

void SliceAnalyzer::AddData(std::map<std::string, uint64_t> &selfTimeKeyValue, const std::string &name,
    uint64_t tmpSelfTime)
{
    if (selfTimeKeyValue.find(name) != selfTimeKeyValue.end()) {
        selfTimeKeyValue.at(name) = selfTimeKeyValue.at(name) + tmpSelfTime;
    } else {
        selfTimeKeyValue.emplace(name, tmpSelfTime);
    }
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
