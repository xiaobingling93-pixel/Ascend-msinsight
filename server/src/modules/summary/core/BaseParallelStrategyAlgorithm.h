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

    virtual void UpdateParallelDimension(const std::string& dimension) = 0;
    virtual void GetArrangementByView() = 0;
protected:
    ParallelStrategyConfig config;
};
}
#endif // PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_BASEPARALLELSTRATEGYALGORITHM_H
