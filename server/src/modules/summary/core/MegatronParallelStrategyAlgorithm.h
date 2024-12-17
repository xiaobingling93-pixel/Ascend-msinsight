/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H
#define PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H

#include "BaseParallelStrategyAlgorithm.h"

namespace Dic::Module {
class MegatronParallelStrategyAlgorithm : public BaseParallelStrategyAlgorithm {
public:
    MegatronParallelStrategyAlgorithm();
    ~MegatronParallelStrategyAlgorithm() override;

    void UpdateParallelDimension(const std::string& dimension) override;
    void GetArrangementByView() override;
private:
    uint32_t elementSize = 1;
    std::string dimension;
};
}
#endif // PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_CORE_MEGATRONPARALLELSTRATEGYALGORITHM_H
