/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "SliceDepthCacheManager.h"
namespace Dic {
namespace Module {
namespace Timeline {
Dic::Module::Timeline::SliceDepthCacheManager &Dic::Module::Timeline::SliceDepthCacheManager::Instance()
{
    static SliceDepthCacheManager sliceDepthCacheManager;
    return sliceDepthCacheManager;
}
}
}
}