/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_BASEPARALLELSTRATEGYALGORITHM_H
#define PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_BASEPARALLELSTRATEGYALGORITHM_H

#include <unordered_map>
#include "ClusterDef.h"
#include "SummaryProtocolRequest.h"

namespace Dic::Module {
const std::vector<std::string> LAYOUT = {DP_PARA, CP_PARA, PP_PARA, TP_PARA};
class BaseParallelStrategyAlgorithm {
public:
    BaseParallelStrategyAlgorithm() = default;
    virtual ~BaseParallelStrategyAlgorithm() = default;

    void SetStrategyConfig(const ParallelStrategyConfig& config);
    virtual void ClearStrategyConfigCache() = 0;
    virtual bool UpdateParallelDimension(const std::string &dimension,
                                         const ParallelStrategyConfig &tmpConfig, std::string &err) = 0;
    virtual void GenerateArrangementByDimension() = 0;
    virtual ArrangementAndConnectionData GetArrangementData() = 0;
    virtual bool GetPerformanceIndicatorByDimension(const Protocol::ParallelismPerformance &config,
        const std::unordered_map<std::uint32_t, StepStatistic> &statistic, PerformanceIndicatorData &data,
        std::string &err) = 0;

protected:
    ParallelStrategyConfig strategyConfig;
    int64_t GetParallelSizeByType(const std::string& type) const;
};
}
#endif // PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_BASEPARALLELSTRATEGYALGORITHM_H
