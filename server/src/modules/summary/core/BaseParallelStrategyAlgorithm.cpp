/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "BaseParallelStrategyAlgorithm.h"

namespace Dic::Module {
void BaseParallelStrategyAlgorithm::SetStrategyConfig(const ParallelStrategyConfig& config)
{
    strategyConfig = config;
}
}
