/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SUMMARYSERVICE_H
#define PROFILER_SERVER_SUMMARYSERVICE_H
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "ParallelStrategyAlgorithmManager.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Protocol;

class SummaryService {
public:
    static bool CheckTp2DSizeForMindSpeed(const ParallelStrategyConfig& config, std::string& errorMsg);
    static bool CheckParamForMindSpeed(const ParallelStrategyConfig& config, std::string& err);
    static void QueryCompareSummaryBaseInfo(const SummaryTopRankRequest &request, SummaryTopRankResponse &response);
    static bool QuerySummaryBaseInfo(SummaryBaseInfo &baseInfo, std::shared_ptr<VirtualClusterDatabase> &db);
    static bool QueryParallelismPerformanceInfo(const ParallelismPerformance &params,
                                                PerformanceIndicatorData &indicatorData);
    static std::unordered_map<std::string, std::vector<CommInfoUnderRank>> QueryParallelismCommTime(
        std::shared_ptr<VirtualClusterDatabase> &database, const GetPerformanceIndicatorParam &params);
private:
    static std::vector<IndicatorDataStruct> GetPerformanceDataByDimension(
        std::shared_ptr<VirtualClusterDatabase> &database, const GetPerformanceIndicatorParam &params);
    static void MergeParallelismPerformance(std::vector<IndicatorDataStruct> &compare,
                                            std::vector<IndicatorDataStruct> &baseline,
                                            PerformanceIndicatorData &indicatorData);
    static std::unordered_map<std::string, double> CalDiffIndicators(std::unordered_map<std::string, double> &compare,
                                                                     std::unordered_map<std::string, double> &baseline);
    static std::unordered_map<std::string, std::vector<CommInfoUnderRank>> MatchCommDataForConnection(
        const std::vector<CommInfoUnderRank> &commTimeForRankDim, const std::vector<Connection> &connections,
        const std::vector<std::string> &importRankList);
    static bool IsConnectionMappingToGroup(const std::string &group, const std::vector<std::string> &fullRankList,
                                           const std::vector<std::string> &importRankList);
    static void MergeCommDataPerformance(std::unordered_map<std::string, std::vector<CommInfoUnderRank>> &compare,
                                         std::unordered_map<std::string, std::vector<CommInfoUnderRank>> &baseline,
                                         PerformanceIndicatorData &indicatorData);
    static void MergeCommInfo(std::vector<CommInfoUnderRank> compare, std::vector<CommInfoUnderRank> baseline,
                              CompareData<std::unordered_map<std::string, double>> &commRes);
    static inline int numberThousands = 1000;
};
}
}
}

#endif // PROFILER_SERVER_SUMMARYSERVICE_H
