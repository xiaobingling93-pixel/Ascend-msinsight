/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "BaseParallelStrategyAlgorithm.h"

namespace Dic::Module {
void BaseParallelStrategyAlgorithm::SetStrategyConfig(const ParallelStrategyConfig& config)
{
    strategyConfig = config;
}

int64_t BaseParallelStrategyAlgorithm::GetParallelSizeByType(const std::string& type) const
{
    if (type == DP_PARA) {
        return strategyConfig.dpSize;
    } else if (type == EP_PARA) {
        return strategyConfig.epSize;
    } else if (type == PP_PARA) {
        return strategyConfig.ppSize;
    } else if (type == TP_PARA) {
        return strategyConfig.tpSize;
    } else if (type == CP_PARA) {
        return strategyConfig.cpSize;
    }
    // 默认值为1，表征没有启用对应的并行方式
    return 1;
}
}
