/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "SummaryService.h"
#include "TraceTime.h"
#include "DataBaseManager.h"
#include "NumberUtil.h"

namespace Dic {
namespace Module {
namespace Summary {
bool SummaryService::QuerySummaryBaseInfo(SummaryBaseInfo &baseInfo, std::shared_ptr<VirtualClusterDatabase> &db)
{
    // db在外层进行校验 必不为空
    if (!db->QueryBaseInfo(baseInfo)) {
        ServerLog::Warn("Fail to query summary base info.");
        return false;
    }
    uint64_t min = UINT64_MAX;
    uint64_t max = 0;
    db->QueryExtremumTimestamp(min, max);
    if (min > max) {
        ServerLog::Warn("Fail to get extremum timestamp when query summary base info.");
        return false;
    }
    baseInfo.collectStartTime =
        NumberUtil::CeilingClamp(min / (numberThousands * numberThousands), (uint64_t)INT64_MAX);
    baseInfo.collectDuration = NumberUtil::CeilingClamp((max - min) / numberThousands, (uint64_t)INT64_MAX);
    return true;
}
void SummaryService::QueryCompareSummaryBaseInfo(const SummaryTopRankRequest &request, SummaryTopRankResponse &response)
{
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr || !QuerySummaryBaseInfo(response.body.baseInfo.compare, database)) {
        ServerLog::Warn("Fail to query compare summary base info");
    }

    if (!request.params.isCompare) {
        return;
    }

    auto baselineDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(BASELINE);
    if (baselineDatabase == nullptr || !QuerySummaryBaseInfo(response.body.baseInfo.baseline, baselineDatabase)) {
        ServerLog::Warn("Fail to query baseline summary base info");
    }
}

std::vector<IndicatorDataStruct> SummaryService::GetPerformanceDataByDimension(
    std::shared_ptr<VirtualClusterDatabase> &database,  const GetPerformanceIndicatorParam &params)
{
    std::vector<IndicatorDataStruct> indicatorData;
    if (database == nullptr) {
        ServerLog::Warn("Fail to query compare parallelism info");
        return indicatorData;
    }
    std::unordered_map<std::uint32_t, StepStatistic> stepStatisticData{};
    bool result = database->QueryAllPerformanceDataByStep(params.step, stepStatisticData);
    if (!result || stepStatisticData.empty()) {
        ServerLog::Warn("Failed to query parallelism performance data.");
        return indicatorData;
    }
    std::string err;
    auto algPtr = ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName(database->GetDbPath(), err);
    if (algPtr == nullptr) {
        ServerLog::Warn("Failed to get algorithm by project name for query parallelism performance.");
        return indicatorData;
    }
    result = algPtr->GetPerformanceIndicatorByDimension(params, stepStatisticData, indicatorData, err);
    if (!result) {
        ServerLog::Warn(err);
        return indicatorData;
    }
    return indicatorData;
}

std::unordered_map<std::string, double> SummaryService::CalDiffIndicators(
    std::unordered_map<std::string, double> &compare, std::unordered_map<std::string, double> &baseline)
{
    std::set<std::string> keySet;
    for (const auto &item: compare) {
        keySet.insert(item.first);
    }
    for (const auto &item: baseline) {
        keySet.insert(item.first);
    }
    std::unordered_map<std::string, double> diff;
    // 数据保留三位小数
    int precision = 3;
    for (const auto &item: keySet) {
        diff[item] = NumberUtil::DoubleReservedNDigits(compare[item] - baseline[item], precision);
    }
    return diff;
}

void SummaryService::MergeParallelismPerformance(std::vector<IndicatorDataStruct> &compare,
                                                 std::vector<IndicatorDataStruct> &baseline,
                                                 PerformanceIndicatorData &indicatorData)
{
    std::set<uint32_t> indexList;
    std::map<uint32_t, IndicatorDataStruct> compareMap;
    for (const auto &item: compare) {
        indexList.insert(item.index);
        compareMap[item.index] = item;
    }

    std::map<uint32_t, IndicatorDataStruct> baselineMap;
    for (const auto &item: baseline) {
        indexList.insert(item.index);
        baselineMap[item.index] = item;
    }

    for (const auto &item: indexList) {
        IndicatorDataStructVo indicatorDataStructVo;
        indicatorDataStructVo.index = item;
        if (compareMap.count(item)) {
            indicatorDataStructVo.indicators.compare = compareMap[item].indicators;
        }
        if (baselineMap.count(item)) {
            indicatorDataStructVo.indicators.baseline = baselineMap[item].indicators;
        }
        indicatorDataStructVo.indicators.diff = CalDiffIndicators(indicatorDataStructVo.indicators.compare,
                                                                  indicatorDataStructVo.indicators.baseline);
        indicatorData.performanceData.push_back(indicatorDataStructVo);
    }
}

bool SummaryService::QueryParallelismPerformanceInfo(const ParallelismPerformance &params,
                                                     PerformanceIndicatorData &indicatorData)
{
    // 查询compare数据
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    GetPerformanceIndicatorParam indicatorParam{params.step,  params.dimension, params.config};
    std::vector<IndicatorDataStruct> compareIndicatorData = GetPerformanceDataByDimension(database,
                                                                                          indicatorParam);
    // 查询baseline数据
    std::vector<IndicatorDataStruct> baselineIndicatorData;
    if (params.isCompare) {
        auto databaseBaseline = Timeline::DataBaseManager::Instance().GetClusterDatabase(BASELINE);
        GetPerformanceIndicatorParam baselineParams{params.baselineStep, params.dimension, params.config};
        baselineIndicatorData = GetPerformanceDataByDimension(databaseBaseline, baselineParams);
    }

    if (compareIndicatorData.empty() && baselineIndicatorData.empty()) {
        ServerLog::Error("Fail to query parallelism performance info.");
        return false;
    }
    MergeParallelismPerformance(compareIndicatorData, baselineIndicatorData, indicatorData);
    // 非对比状态下，才计算专家建议信息
    if (!params.isCompare && database != nullptr) {
        std::string err;
        auto algPtr =
            ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName(database->GetDbPath(), err);
        if (algPtr != nullptr) {
            algPtr->CalAdviceInfo(params.dimension, indicatorData.advices, compareIndicatorData);
        }
    }
    return true;
}

}
}
}
