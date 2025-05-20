/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "SummaryService.h"
#include "TraceTime.h"
#include "DataBaseManager.h"
#include "CollectionUtil.h"
#include "BaselineManager.h"

using namespace Dic::Module::Summary;
using namespace Dic::Module::Global;
using namespace Dic::Protocol;
using namespace Dic::Module;
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
        NumberUtil::CeilingClamp(min / (numberThousands * numberThousands), static_cast<uint64_t>(INT64_MAX));
    baseInfo.collectDuration =
        NumberUtil::CeilingClamp((max - min) / numberThousands, static_cast<uint64_t>(INT64_MAX));
    return true;
}
void SummaryService::QueryCompareSummaryBaseInfo(const SummaryTopRankRequest &request, SummaryTopRankResponse &response)
{
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(request.params.clusterPath);
    if (database == nullptr || !QuerySummaryBaseInfo(response.body.baseInfo.compare, database)) {
        ServerLog::Warn("Fail to query compare summary base info");
    }

    if (!request.params.isCompare) {
        return;
    }

    auto baselineDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(
        BaselineManager::Instance().GetBaseLineClusterPath());
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
        ServerLog::Warn("Failed to query original parallelism performance data.");
        return indicatorData;
    }
    auto algPtr = ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName(database->GetDbPath());
    if (algPtr == nullptr) {
        ServerLog::Warn("Failed to get algorithm by project name for query parallelism performance.");
        return indicatorData;
    }
    std::string err;
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
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(params.clusterPath);
    GetPerformanceIndicatorParam indicatorParam{params.step,  params.dimension, params.config};
    std::vector<IndicatorDataStruct> compareIndicatorData = GetPerformanceDataByDimension(database,
                                                                                          indicatorParam);
    std::unordered_map<std::string, std::vector<CommInfoUnderRank>> compareCommInfo =
        QueryParallelismCommTime(database, indicatorParam);
    // 查询baseline数据
    std::vector<IndicatorDataStruct> baselineIndicatorData;
    std::unordered_map<std::string, std::vector<CommInfoUnderRank>> baselineCommInfo;
    if (params.isCompare) {
        auto databaseBaseline = Timeline::DataBaseManager::Instance().GetClusterDatabase(
            BaselineManager::Instance().GetBaseLineClusterPath());
        GetPerformanceIndicatorParam baselineParams{params.baselineStep, params.dimension, params.config};
        baselineIndicatorData = GetPerformanceDataByDimension(databaseBaseline, baselineParams);
        baselineCommInfo = QueryParallelismCommTime(databaseBaseline, baselineParams);
    }

    if (compareIndicatorData.empty() && baselineIndicatorData.empty()) {
        ServerLog::Error("Fail to query parallelism performance info.");
        return false;
    }
    MergeParallelismPerformance(compareIndicatorData, baselineIndicatorData, indicatorData);
    MergeCommDataPerformance(compareCommInfo, baselineCommInfo, indicatorData);
    // 非对比状态下，才计算专家建议信息
    if (!params.isCompare && database != nullptr) {
        auto algPtr =
            ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName(database->GetDbPath());
        if (algPtr != nullptr) {
            algPtr->CalAdviceInfo(params.dimension, indicatorData.advices, compareIndicatorData);
        }
    }
    return true;
}

void SummaryService::MergeCommDataPerformance(std::unordered_map<std::string, std::vector<CommInfoUnderRank>> &compare,
    std::unordered_map<std::string, std::vector<CommInfoUnderRank>> &baseline, PerformanceIndicatorData &indicatorData)
{
    for (auto &item: indicatorData.performanceData) {
        std::string key = std::to_string(item.index);
        MergeCommInfo(compare[key], baseline[key], item.commTimeIndicator);
    }
}

void SummaryService::MergeCommInfo(std::vector<CommInfoUnderRank> compare, std::vector<CommInfoUnderRank> baseline,
                                   CompareData<std::unordered_map<std::string, double>> &commRes)
{
    std::unordered_map<std::string, double> compareMap;
    for (const auto &item: compare) {
        compareMap[item.pgName] = item.commTime;
    }

    std::unordered_map<std::string, double> baselineMap;
    for (const auto &item: baseline) {
        baselineMap[item.pgName] = item.commTime;
    }

    commRes.compare = compareMap;
    commRes.baseline = baselineMap;
    commRes.diff = CalDiffIndicators(compareMap, baselineMap);
}

std::unordered_map<std::string, std::vector<CommInfoUnderRank>> SummaryService::QueryParallelismCommTime(
    std::shared_ptr<VirtualClusterDatabase> &database, const GetPerformanceIndicatorParam &params)
{
    if (database == nullptr) {
        ServerLog::Warn("Fail to query parallelism communication info, database not exist.");
        return {};
    }
    auto algPtr =
            ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName(database->GetDbPath());
    if (algPtr == nullptr) {
        ServerLog::Warn("Failed to get algorithm by project name for query parallelism communication info.");
        return {};
    }
    std::string err;
    // 获取根据并行策略配置计算出来的通信域信息
    std::vector<Connection> connections = algPtr->GetAllCommunicationGroups(err);
    if (!err.empty()) {
        // 完整链接数据不存在，则直接返回
        ServerLog::Warn("Fail to get all communication groups info.");
        return {};
    }
    // 获取导入的rank数据信息
    std::vector<std::string> importRankList = database->GetAllRankFromStepStatisticInfo();
    if (importRankList.empty()) {
        ServerLog::Warn("Fail to get all rank from step statistic info.");
        return {};
    }

    // 获取要填充的数据
    std::vector<CommInfoUnderRank> commTimeForRankDim = database->GetCommTimeForRankDim(params.step);
    if (commTimeForRankDim.empty()) {
        ServerLog::Warn("Fail to get communication time data.");
        return {};
    }
    // 匹配数据
    auto MatchRes = MatchCommDataForConnection(commTimeForRankDim, connections, importRankList);
    // 数据根据维度进行折叠
    return algPtr->GetCommInfoByDimension(MatchRes, params.dimension);
}

/**
 * 将communication通信数据与Connection（summary中计算出来的链接数据）进行匹配
 * @param commTimeForRankDim 从通信耗时表查询到的数据，每个通信域下每个卡的通信数据
 * @param connections 按照并行策略计算出的全展开视图
 * @param importRankList 实际导入的rank列表
 * @return 匹配结果
 */
std::unordered_map<std::string, std::vector<CommInfoUnderRank>> SummaryService::MatchCommDataForConnection(
    const std::vector<CommInfoUnderRank> &commTimeForRankDim, const std::vector<Connection> &connections,
    const std::vector<std::string> &importRankList)
{
    std::unordered_map<std::string, std::vector<CommInfoUnderRank>> commTimeMap;
    for (const auto &item: commTimeForRankDim) {
        std::vector<std::string> groupList = StringUtil::SplitStringWithParenthesesByComma(item.rankSet);
        std::sort(groupList.begin(), groupList.end());
        std::string rankSortStr = StringUtil::join(groupList, '-');
        commTimeMap[rankSortStr].push_back(item);
    }
    // 数据分配，为每一个connection找对应的通信数据
    std::unordered_map<std::string, std::vector<CommInfoUnderRank>> res;
    for (const auto &item: connections) {
        std::vector<std::string> fullRankList;
        std::transform(item.indexes.begin(), item.indexes.end(), std::back_inserter(fullRankList), [](uint32_t num) {
            return std::to_string(num);
        });
        std::vector<std::string> aclRankList = CollectionUtil::CalIntersection(fullRankList, importRankList);
        std::sort(aclRankList.begin(), aclRankList.end());
        std::string rankSortStr = StringUtil::join(aclRankList, '-');
        auto findRes = commTimeMap.find(rankSortStr);
        if (findRes != commTimeMap.end()) {
            for (auto commItem: findRes->second) {
                commItem.pgName = item.type;
                res[commItem.rankId].push_back(commItem);
            }
        }
    }
    return res;
}