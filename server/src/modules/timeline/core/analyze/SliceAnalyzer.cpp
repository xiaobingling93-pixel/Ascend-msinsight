/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "SliceAnalyzer.h"
namespace Dic::Module::Timeline {
SliceAnalyzer::SliceAnalyzer()
{
    if (repository == nullptr) {
        repository = std::make_unique<Repository>();
    }
};

void SliceAnalyzer::SetRepository(std::unique_ptr<RepositoryInterface> repositoryDependency)
{
    repository = std::move(repositoryDependency);
}

SliceAnalyzer::~SliceAnalyzer()
{
    if (repository != nullptr) {
        repository = nullptr;
    }
};

std::set<std::pair<uint64_t, uint32_t>> SliceAnalyzer::ComputeResultIds(uint64_t startTime, uint64_t endTime,
    std::vector<SliceDomain> &sliceDomain, std::vector<DepthHelper> &endList,
    const std::vector<uint64_t> &pythonFunctionIds)
{
    // 根据开始时间结束时间把屏幕平均分成1000份
    const int maxDataCount = 1000;
    uint64_t unitTime = (endTime - startTime) / maxDataCount;
    if (unitTime == 0) {
        return ComputeSmallScreenIds(startTime, endTime, sliceDomain, endList, pythonFunctionIds);
    }
    std::set<std::pair<uint64_t, uint32_t>> ids;
    for (auto &item : sliceDomain) {
        if (!std::empty(pythonFunctionIds) &&
            std::binary_search(pythonFunctionIds.begin(), pythonFunctionIds.end(), item.id)) {
            continue;
        }
        for (item.depth = 0; item.depth < endList.size() && endList[item.depth].endTime > item.timestamp;
            item.depth++) {
        }
        if (item.depth < endList.size()) {
            endList[item.depth].endTime = item.endTime;
            // 不在屏幕中的算子只参与深度计算，不参与采样过程
            if (!(item.endTime >= startTime && item.timestamp <= endTime)) {
                continue;
            }
            // 算子开始时间大于当前份屏幕时间，则把tempId加进结果集，重置tempId，进入下一份屏幕采样
            if (item.timestamp > endList[item.depth].curLimitTime && endList[item.depth].curLimitTime <= endTime) {
                ids.emplace(endList[item.depth].tempId, item.depth);
                endList[item.depth].tempId = 0;
                endList[item.depth].tempDuration = 0;
                endList[item.depth].curLimitTime = item.timestamp + unitTime;
            }
            // 更新tempId
            if (endList[item.depth].tempDuration <= item.endTime - item.timestamp) {
                endList[item.depth].tempId = item.id;
                endList[item.depth].tempDuration = item.endTime - item.timestamp;
            }
        } else {
            DepthHelper depthHelper;
            depthHelper.endTime = item.endTime;
            depthHelper.curLimitTime = startTime + unitTime;
            if (item.endTime >= startTime && item.timestamp <= endTime) {
                depthHelper.tempId = item.id;
                depthHelper.tempDuration = item.endTime - item.timestamp;
            }
            endList.emplace_back(depthHelper);
        }
    }
    for (size_t i = 0; i < endList.size(); ++i) {
        ids.emplace(endList[i].tempId, i);
    }
    return ids;
}

/**
 * 屏幕范围小于1000ns的计算方式
 * @param startTime
 * @param endTime
 * @param sliceDomain
 * @param endList
 * @param pythonFunctionIds
 * @return
 */
std::set<std::pair<uint64_t, uint32_t>> SliceAnalyzer::ComputeSmallScreenIds(uint64_t startTime, uint64_t endTime,
    std::vector<SliceDomain> &sliceDomain, std::vector<DepthHelper> &endList,
    const std::vector<uint64_t> &pythonFunctionIds)
{
    std::set<std::pair<uint64_t, uint32_t>> ids;
    for (auto &item : sliceDomain) {
        if (!std::empty(pythonFunctionIds) &&
            std::binary_search(pythonFunctionIds.begin(), pythonFunctionIds.end(), item.id)) {
            continue;
        }
        for (item.depth = 0; item.depth < endList.size() && endList[item.depth].endTime > item.timestamp;
            item.depth++) {
        }
        if (item.depth < endList.size()) {
            endList[item.depth].endTime = item.endTime;
        } else {
            DepthHelper depthHelper;
            depthHelper.endTime = item.endTime;
            endList.emplace_back(depthHelper);
        }
        if (item.endTime >= startTime && item.timestamp <= endTime) {
            ids.emplace(item.id, item.depth);
        }
    }
    return ids;
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
 */
void SliceAnalyzer::CalculateSelfTime(std::vector<CompeteSliceDomain> &rows,
    std::map<std::string, uint64_t> &selfTimeKeyValue)
{
    size_t length = rows.size();
    // offset变量用来优化性能
    uint64_t offset = 0;
    for (size_t i = 0; i < length; i++) {
        uint32_t curDepth = rows[i].depth;
        uint64_t selfTime = rows[i].duration;
        uint64_t curSliceStartTime = rows[i].timestamp;
        uint64_t curSliceEndTime = rows[i].endTime;
        for (uint64_t j = offset; j < length; ++j) {
            if (j == length - 1 && rows[j].depth == curDepth) {
                offset = length;
                continue;
            }
            if (rows[j].depth < curDepth + 1) {
                continue;
            }
            if (rows[j].depth > curDepth + 1) {
                offset = j;
                break;
            }
            if (rows[j].timestamp < curSliceStartTime) {
                continue;
            }
            if (rows[j].endTime > curSliceEndTime) {
                offset = j;
                break;
            }
            selfTime = selfTime - rows[j].duration;
            offset = j;
        }
        AddData(selfTimeKeyValue, rows[i].name, selfTime);
    }
}


void SliceAnalyzer::ComputeScreenSliceIds(const SliceQuery &sliceQuery, std::set<uint64_t> &ids, uint64_t &maxDepth,
    bool &havePythonFunction, std::map<uint64_t, int32_t> &depthMap)
{
    std::string sliceCacheKey = std::to_string(sliceQuery.trackId);
    auto &instance = SliceCacheManager::Instance();
    instance.UpdatePythonFilterSet(sliceCacheKey, sliceQuery.isFilterPythonFunction);
    std::vector<SliceDomain> sliceDomainVec = instance.GetSliceDomainVec(sliceCacheKey);
    if (std::empty(sliceDomainVec)) {
        repository->QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceDomainVec);
    }
    std::vector<uint64_t> pythonFunctionIds;
    QueryPythonFuncIds(sliceQuery, pythonFunctionIds);
    std::vector<DepthHelper> endList;
    std::set<std::pair<uint64_t, uint32_t>> idPairVec = ComputeResultIds(sliceQuery.startTime + sliceQuery.minTimestamp,
        sliceQuery.endTime + sliceQuery.minTimestamp, sliceDomainVec, endList, pythonFunctionIds);
    for (const auto &item : idPairVec) {
        ids.emplace(item.first);
        depthMap[item.first] = item.second;
    }
    maxDepth = endList.size();
    havePythonFunction = instance.HavePythonFunction(sliceQuery.trackId);
    instance.UpdateSliceCache(sliceCacheKey, sliceDomainVec);
}


void SliceAnalyzer::QueryPythonFuncIds(const SliceQuery &sliceQuery, std::vector<uint64_t> &pythonFunctionIds)
{
    auto &instance = SliceCacheManager::Instance();
    std::string sliceCacheKey = std::to_string(sliceQuery.trackId);
    if (instance.HavePythonFunction(sliceQuery.trackId)) {
        uint64_t count = repository->QueryPythonFunctionCountByTrackId(sliceQuery);
        if (count == 0) {
            instance.SetPythonFunctionStatus(sliceQuery.trackId, false);
        }
    }
    if (sliceQuery.isFilterPythonFunction && instance.HavePythonFunction(sliceQuery.trackId)) {
        pythonFunctionIds = instance.GetPythonFunctionIdVec(sliceCacheKey);
        if (std::empty(pythonFunctionIds)) {
            repository->QuerySliceIdsByCat(sliceQuery, pythonFunctionIds);
            instance.PutPythonFunctionIdVec(sliceCacheKey, pythonFunctionIds);
        }
    }
}

void SliceAnalyzer::ComputeSliceDomainVecAndSelfTimeByTimeRange(const SliceQuery &sliceQuery,
    std::vector<CompeteSliceDomain> &sliceDomainVec, std::map<std::string, uint64_t> &selfTimeKeyValue)
{
    std::vector<CompeteSliceDomain> allCompeteSliceVec;
    // 查询符合条件的所有算子
    repository->QueryCompeteSliceVecByTimeRangeAndTrackId(sliceQuery, allCompeteSliceVec);
    if (std::empty(allCompeteSliceVec)) {
        return;
    }
    // 过滤python function
    std::vector<CompeteSliceDomain> competeSliceVec;
    std::string sliceCacheKey = std::to_string(sliceQuery.trackId);
    auto &instance = SliceCacheManager::Instance();
    std::vector<uint64_t> pythonFunctionIds = instance.GetPythonFunctionIdVec(sliceCacheKey);
    std::unordered_map<uint64_t, uint32_t> depthInfo;
    ComputeDepthInfoByTrackId(sliceQuery, depthInfo);
    // 过滤python function
    for (auto &item : allCompeteSliceVec) {
        if (std::binary_search(pythonFunctionIds.begin(), pythonFunctionIds.end(), item.id)) {
            continue;
        }
        item.depth = depthInfo[item.id];
        competeSliceVec.emplace_back(std::move(item));
    }
    std::sort(competeSliceVec.begin(), competeSliceVec.end(), std::less<CompeteSliceDomain>());
    CalculateSelfTime(competeSliceVec, selfTimeKeyValue);
    uint64_t end = sliceQuery.endTime + sliceQuery.minTimestamp;
    uint64_t start = sliceQuery.startTime + sliceQuery.minTimestamp;
    for (auto &row : competeSliceVec) {
        if (row.timestamp <= end && row.endTime >= start) {
            sliceDomainVec.emplace_back(row);
        }
    }
}

void SliceAnalyzer::ComputeDepthInfoByTrackId(const SliceQuery &sliceQuery,
    std::unordered_map<uint64_t, uint32_t> &depthInfo)
{
    SliceCacheManager &sliceCacheManager = SliceCacheManager::Instance();
    bool cacheIsExist = sliceCacheManager.QueryDepthInfo(std::to_string(sliceQuery.trackId), depthInfo);
    if (!cacheIsExist) {
        ComputeDepthInfoFromDB(sliceQuery, depthInfo);
    }
}

void SliceAnalyzer::ComputeSliceDomainVecByTrackId(const SliceQuery &sliceQuery, std::vector<SliceDomain> &sliceVec)
{
    SliceCacheManager &sliceCacheManager = SliceCacheManager::Instance();
    sliceVec = sliceCacheManager.GetSliceDomainVec(std::to_string(sliceQuery.trackId));
    if (std::empty(sliceVec)) {
        std::unordered_map<uint64_t, uint32_t> depthInfo;
        ComputeDepthInfoFromDB(sliceQuery, depthInfo);
        sliceVec = sliceCacheManager.GetSliceDomainVec(std::to_string(sliceQuery.trackId));
    }
}

void SliceAnalyzer::ComputeDepthInfoFromDB(const SliceQuery &sliceQuery,
    std::unordered_map<uint64_t, uint32_t> &depthInfo)
{
    std::vector<SliceDomain> sliceVec;
    SliceCacheManager &simpleSliceCache = SliceCacheManager::Instance();
    std::string pythonFunctionKey = std::to_string(sliceQuery.trackId);
    std::vector<uint64_t> pythonFunctionIds = simpleSliceCache.GetPythonFunctionIdVec(pythonFunctionKey);
    repository->QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceVec);
    std::vector<uint64_t> endList;
    for (auto &item : sliceVec) {
        if (std::binary_search(pythonFunctionIds.begin(), pythonFunctionIds.end(), item.id)) {
            continue;
        }
        while (item.depth < endList.size() && endList[item.depth] > item.timestamp) {
            item.depth++;
        }
        if (item.depth < endList.size()) {
            endList[item.depth] = item.endTime;
        } else {
            endList.emplace_back(item.endTime);
        }
        depthInfo[item.id] = item.depth;
    }
    simpleSliceCache.UpdateSliceCache(std::to_string(sliceQuery.trackId), sliceVec);
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

void SliceAnalyzer::ComputeAllThreadInfo(const ThreadQuery &flowQuery,
    std::unordered_map<uint64_t, std::pair<std::string, std::string>> &threadInfo)
{
    repository->QueryAllThreadInfo(flowQuery, threadInfo);
}
}
