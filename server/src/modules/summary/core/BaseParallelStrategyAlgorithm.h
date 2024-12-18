/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_BASEPARALLELSTRATEGYALGORITHM_H
#define PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_BASEPARALLELSTRATEGYALGORITHM_H

#include "ClusterDef.h"

namespace Dic::Module {
class BaseParallelStrategyAlgorithm {
public:
    BaseParallelStrategyAlgorithm() = default;
    virtual ~BaseParallelStrategyAlgorithm() = default;

    virtual bool UpdateParallelDimension(const std::string &dimension,
                                         const ParallelStrategyConfig &tmpConfig, std::string &err) = 0;
    virtual void GenerateArrangementByDimension() = 0;
    virtual ArrangementAndConnectionData GetArrangementData() = 0;
protected:
    ParallelStrategyConfig config;
};
}
#endif // PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_BASEPARALLELSTRATEGYALGORITHM_H
