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
    for (int i = 0; i < endList.size(); ++i) {
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
            if (item.endTime >= startTime && item.timestamp <= endTime) {
                ids.emplace(item.id, item.depth);
            }
        } else {
            DepthHelper depthHelper;
            depthHelper.endTime = item.endTime;
            endList.emplace_back(depthHelper);
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


void SliceAnalyzer::ComputeScreenSliceIds(const SliceQuery &sliceQuery, std::set<uint64_t> &ids, uint64_t &maxDepth,
    bool &havePythonFunction, std::map<uint64_t, int32_t> &depthMap)
{
    // 泳道下数据量大于10万则放入缓存，否则仍从数据库查询,这里20万改为10万是因为从数据库查询20万数据时间略长
    constexpr int minCacheSizeLimit = 100000;
    std::string sliceCacheKey =
        sliceQuery.cardId + "_" + std::to_string(sliceQuery.trackId) + "_SliceAnalyzer::ComputeScreenSliceIds";
    auto &instance = CacheManager::Instance();
    std::vector<SliceDomain> sliceDomainVec = instance.GetSliceDomainVec(sliceCacheKey);
    if (std::empty(sliceDomainVec)) {
        repository->QuerySimpleSliceWithOutNameByTrackId(sliceQuery, sliceDomainVec);
        if (sliceDomainVec.size() > minCacheSizeLimit) {
            instance.Put(sliceCacheKey, sliceDomainVec);
        }
    }

    if (instance.HavePythonFunction(sliceQuery.trackId)) {
        uint64_t count = repository->QueryPythonFunctionCountByTrackId(sliceQuery);
        if (count == 0) {
            instance.SetPythonFunctionStatus(sliceQuery.trackId, false);
        }
    }
    std::vector<uint64_t> pythonFunctionIds;
    if (sliceQuery.isFilterPythonFunction && instance.HavePythonFunction(sliceQuery.trackId)) {
        std::string pythonFunctionKey =
            sliceQuery.cardId + "_" + std::to_string(sliceQuery.trackId) + "_SliceAnalyzer::PythonFunctionIds";
        pythonFunctionIds = instance.GetPythonFunctionIdVec(pythonFunctionKey);
        if (std::empty(pythonFunctionIds)) {
            repository->QuerySliceIdsByCat(sliceQuery, pythonFunctionIds);
            instance.PutPythonFunctionIdVec(pythonFunctionKey, pythonFunctionIds);
        }
    }
    std::vector<DepthHelper> endList;
    std::set<std::pair<uint64_t, uint32_t>> idPairVec = ComputeResultIds(sliceQuery.startTime + sliceQuery.minTimestamp,
        sliceQuery.endTime + sliceQuery.minTimestamp, sliceDomainVec, endList, pythonFunctionIds);
    for (const auto &item : idPairVec) {
        ids.emplace(item.first);
        depthMap[item.first] = item.second;
    }
    maxDepth = endList.size();
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
