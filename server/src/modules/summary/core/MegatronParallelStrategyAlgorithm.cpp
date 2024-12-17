/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "MegatronParallelStrategyAlgorithm.h"

namespace Dic::Module {
void MegatronParallelStrategyAlgorithm::UpdateParallelDimension(const std::string& tmpDimension)
{
    dimension = tmpDimension;
}
MegatronParallelStrategyAlgorithm::MegatronParallelStrategyAlgorithm()
{
}

MegatronParallelStrategyAlgorithm::~MegatronParallelStrategyAlgorithm()
{
}
void MegatronParallelStrategyAlgorithm::GetArrangementByView() {}
}