/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "SimulationSliceCacheManager.h"
namespace Dic {
namespace Module {
namespace Timeline {
SimulationSliceCacheManager &SimulationSliceCacheManager::Instance()
{
    static SimulationSliceCacheManager simulationSliceCacheManager;
    return simulationSliceCacheManager;
}
}
}
}