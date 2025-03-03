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
private:
    static std::vector<IndicatorDataStruct> GetPerformanceDataByDimension(
        std::shared_ptr<VirtualClusterDatabase> &database, const GetPerformanceIndicatorParam &params);
    static void MergeParallelismPerformance(std::vector<IndicatorDataStruct> &compare,
                                            std::vector<IndicatorDataStruct> &baseline,
                                            PerformanceIndicatorData &indicatorData);
    static std::unordered_map<std::string, double> CalDiffIndicators(std::unordered_map<std::string, double> &compare,
                                                                     std::unordered_map<std::string, double> &baseline);
static inline int numberThousands = 1000;
};
}
}
}

#endif // PROFILER_SERVER_SUMMARYSERVICE_H
