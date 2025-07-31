/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_RLMSTXCONFIGMANAGER_H
#define PROFILER_SERVER_RLMSTXCONFIGMANAGER_H
#include <vector>
#include "RLDomainObject.h"
namespace Dic::Module::RL {
class RLMstxConfigManager {
public:
    static RLMstxConfigManager &Instance();
    RLMstxConfigManager(const RLMstxConfigManager &) = delete;
    RLMstxConfigManager &operator=(const RLMstxConfigManager &) = delete;
    RLMstxConfigManager(RLMstxConfigManager &&) = delete;
    RLMstxConfigManager &operator=(RLMstxConfigManager &&) = delete;

    std::vector<RLMstxConfig> GetRLMstxConfig();
private:
    std::vector<RLMstxConfig> config = {};
    explicit RLMstxConfigManager() = default;
    ~RLMstxConfigManager() = default;
};
}
#endif // PROFILER_SERVER_RLMSTXCONFIGMANAGER_H
