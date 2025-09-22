/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SUMMARYSERVICE_H
#define PROFILER_SERVER_SUMMARYSERVICE_H
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "ParallelStrategyAlgorithmManager.h"
#include "DataBaseManager.h"
#include "ParallelStrategyAlgorithmDef.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Protocol;

class SummaryService {
public:
    static void QueryCompareSummaryBaseInfo(const SummaryTopRankRequest &request, SummaryTopRankResponse &response);
    static bool QuerySummaryBaseInfo(SummaryBaseInfo &baseInfo, std::shared_ptr<VirtualClusterDatabase> &db);
    static bool QueryParallelismPerformanceInfo(const ParallelismPerformance &params,
                                                PerformanceIndicatorData &indicatorData);
    static std::unordered_map<std::string, std::vector<CommInfoUnderRank>> QueryParallelismCommTime(
        const std::shared_ptr<VirtualClusterDatabase> &database, const GetPerformanceIndicatorParam &params,
        CommInfoMap &commInTpDimension);
private:
    static bool UpdateStartTimeAndDuration(SummaryBaseInfo &baseInfo, std::shared_ptr<VirtualClusterDatabase> &db);
    static std::vector<IndicatorDataStruct> GetPerformanceDataByDimension(
        std::shared_ptr<VirtualClusterDatabase> &database, const GetPerformanceIndicatorParam &params);
    static void MergeParallelismPerformance(std::vector<IndicatorDataStruct> &compare,
                                            std::vector<IndicatorDataStruct> &baseline,
                                            PerformanceIndicatorData &indicatorData);
    static std::unordered_map<std::string, double> CalDiffIndicators(std::unordered_map<std::string, double> &compare,
                                                                     std::unordered_map<std::string, double> &baseline);
    static void MergeCommDataPerformance(std::unordered_map<std::string, std::vector<CommInfoUnderRank>> &compare,
                                         std::unordered_map<std::string, std::vector<CommInfoUnderRank>> &baseline,
                                         PerformanceIndicatorData &indicatorData);
    static void MergeCommInfo(std::vector<CommInfoUnderRank> compare, std::vector<CommInfoUnderRank> baseline,
                              CompareData<std::unordered_map<std::string, double>> &commRes);
    static inline int numberThousands = 1000;
    static inline size_t maxRankCountForSummaryWithoutConfig = 64; // 未配置并行策略，最多允许展示64卡的概览
};
}
}
}

#endif // PROFILER_SERVER_SUMMARYSERVICE_H
