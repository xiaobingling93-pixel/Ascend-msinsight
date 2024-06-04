/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "ServerLog.h"
#include "SliceAnalyzer.h"
namespace Dic::Module::Timeline {
SliceAnalyzer::SliceAnalyzer()
{
    if (repository == nullptr) {
        repository = std::make_unique<Repository>();
    }
};

SliceAnalyzer::~SliceAnalyzer()
{
    if (repository != nullptr) {
        repository = nullptr;
    }
};
/**
 * 对前端要展示的算子进行取样
 * @param startTime 页面开始时间
 * @param endTime 页面结束时间
 * @param minTimestamp 最小时间
 * @param cacheSlices 当前泳道内的所有算子数据，按照depth排序，depth相同则按照timestamp排序
 * @return
 */
std::set<uint64_t> SliceAnalyzer::ComputeResultIds(uint64_t startTime, uint64_t endTime,
    std::vector<std::vector<SliceDomain>> &depthSlicesDomainVec)
{
    // 根据开始时间结束时间把屏幕平均分成1000份
    const int maxDataCount = 1000;
    uint64_t unitTime = (endTime - startTime) / maxDataCount;
    std::set<uint64_t> ids;
    for (auto &item : depthSlicesDomainVec) {
        ComputeDepthResultIds(startTime, endTime, item, unitTime, ids);
    }
    return ids;
}

void SliceAnalyzer::ComputeDepthResultIds(uint64_t startTime, uint64_t endTime,
    const std::vector<SliceDomain> &slicesDomainVec, uint64_t unitTime, std::set<uint64_t> &ids)
{
    if (unitTime == 0) {
        ComputeSmallScreenSliceIds(startTime, endTime, slicesDomainVec, ids);
        return;
    }
    // 第一份屏幕的截至时间
    uint64_t curLimitTime = startTime + unitTime;
    uint64_t tempId = 0;
    uint64_t tempDuration = 0;
    for (const auto &item : slicesDomainVec) {
        // 算子结束时间小于屏幕最小时间，过滤
        if (item.endTime < startTime) {
            continue;
        }
        // 算子开始时间大于屏幕最大时间，过滤
        if (item.timestamp > endTime) {
            continue;
        }
        // 算子开始时间大于当前份屏幕时间，则把tempId加进结果集，重置tempId，进入下一份屏幕采样
        while (item.timestamp > curLimitTime && curLimitTime <= endTime) {
            ids.emplace(tempId);
            tempId = 0;
            tempDuration = 0;
            curLimitTime += unitTime;
        }
        // 更新tempId
        if (tempDuration <= item.endTime - item.timestamp) {
            tempId = item.id;
            tempDuration = item.endTime - item.timestamp;
        }
        // 如果算子很长，则找下一份屏幕的截至时间
        while (item.endTime >= curLimitTime && curLimitTime <= endTime) {
            ids.emplace(tempId);
            tempId = 0;
            tempDuration = 0;
            curLimitTime += unitTime;
        }
    }
    ids.emplace(tempId);
}

/**
 * 当屏幕范围小于1000ns时的采集方式
 * @param startTime
 * @param endTime
 * @param minTimestamp
 * @param cacheSlices
 * @param ids
 */
void SliceAnalyzer::ComputeSmallScreenSliceIds(uint64_t startTime, uint64_t endTime,
    const std::vector<SliceDomain> &cacheSlices, std::set<uint64_t> &ids)
{
    for (const auto &item : cacheSlices) {
        // 算子结束时间小于屏幕最小时间或者算子开始时间大于屏幕最大时间，过滤
        if (item.endTime < startTime || item.timestamp > endTime) {
            continue;
        }
        ids.emplace(item.id);
    }
}

void SliceAnalyzer::SortByTimestampASC(std::vector<SliceDomain> &cacheSlices)
{
    std::sort(cacheSlices.begin(), cacheSlices.end(), SliceAnalyzer::CompareTimestampASC);
}

uint32_t SliceAnalyzer::ComputeFlowPointDepth(std::vector<SliceDomain> &cacheSlices, std::string &type,
    uint64_t timestamp)
{
    SliceDomain cacheSlice;
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

void SliceAnalyzer::ComputeDepth(std::vector<SliceDomain> &sliceDomainVec, const std::set<uint64_t> &pythonFunctionIds,
    std::vector<std::vector<SliceDomain>> &depthCacheSlice, std::map<uint64_t, int32_t> &depthMap)
{
    for (auto &item : sliceDomainVec) {
        if (!std::empty(pythonFunctionIds) && pythonFunctionIds.count(item.id) > 0) {
            continue;
        }
        bool isEmplace = false;
        size_t size = depthCacheSlice.size();
        for (int i = 0; i < size; ++i) {
            if (depthCacheSlice[i].back().endTime < item.timestamp) {
                depthMap[item.id] = i;
                depthCacheSlice[i].emplace_back(item);
                isEmplace = true;
                break;
            }
        }
        if (!isEmplace) {
            std::vector<SliceDomain> temp;
            depthMap[item.id] = size;
            temp.emplace_back(item);
            depthCacheSlice.emplace_back(temp);
        }
    }
}

void SliceAnalyzer::ComputeScreenSliceIds(const SliceQuery &sliceQuery, std::set<uint64_t> &ids, uint64_t &maxDepth,
    bool &havePythonFunction, std::map<uint64_t, int32_t> &depthMap)
{
    // 泳道下数据量大于20万则放入缓存，否则仍从数据库查询
    constexpr int minCacheSizeLimit = 200000;
    std::string sliceCacheKey =
        sliceQuery.cardId + "_" + std::to_string(sliceQuery.trackId) + "_SliceAnalyzer::ComputeScreenSliceIds";
    auto &instance = CacheManager::Instance();
    std::vector<SliceDomain> sliceDomainVec = instance.Get(sliceCacheKey);
    std::set<uint64_t> pythonFunctionIds;
    if (instance.HavePythonFunction(sliceQuery.trackId)) {
        repository->QuerySliceIdsByCat(sliceQuery, pythonFunctionIds);
        if (std::empty(pythonFunctionIds)) {
            instance.SetPythonFunctionStatus(sliceQuery.trackId, false);
        }
    }
    if (!sliceQuery.isFilterPythonFunction) {
        pythonFunctionIds.clear();
    }
    if (std::empty(sliceDomainVec)) {
        repository->QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceDomainVec);
        if (sliceDomainVec.size() > minCacheSizeLimit) {
            CacheManager::Instance().Put(sliceCacheKey, sliceDomainVec);
        }
    }
    std::vector<std::vector<SliceDomain>> depthSliceVec;
    ComputeDepth(sliceDomainVec, pythonFunctionIds, depthSliceVec, depthMap);
    ids = ComputeResultIds(sliceQuery.startTime + sliceQuery.minTimestamp, sliceQuery.endTime + sliceQuery.minTimestamp,
        depthSliceVec);
    maxDepth = depthSliceVec.size();
    havePythonFunction = instance.HavePythonFunction(sliceQuery.trackId);
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

bool SliceAnalyzer::CompareTimestampASC(const SliceDomain &first, const SliceDomain &second)
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
