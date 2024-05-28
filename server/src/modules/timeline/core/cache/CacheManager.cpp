/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "CacheManager.h"
namespace Dic {
namespace Module {
namespace Timeline {
Dic::Module::Timeline::CacheManager &Dic::Module::Timeline::CacheManager::Instance()
{
    static CacheManager cacheManager;
    return cacheManager;
}
}
}
}
