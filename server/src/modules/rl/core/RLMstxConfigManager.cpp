/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "RLMstxConfigManager.h"

namespace Dic::Module::RL {
    RLMstxConfigManager &RLMstxConfigManager::Instance()
    {
        static RLMstxConfigManager instance;
        return instance;
    }

    std::vector<RLMstxConfig> RLMstxConfigManager::GetRLMstxConfig()
    {
        return config;
    }
}
