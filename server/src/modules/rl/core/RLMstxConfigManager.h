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
    std::vector<std::string> GetMstxTaskNameList();
    std::string GetTaskTypeByName(const std::string &name);

    /**
     * @brief 根据taskname获取匹配的配置项
     */
    RLMstxConfig GetMstxConfigByTaskName(const std::vector<std::string> &taskNames);
private:
    void InitConfig();
    void InitDefaultConf();

    explicit RLMstxConfigManager();
    ~RLMstxConfigManager() = default;

    std::vector<RLMstxConfig> config;
};
}
#endif // PROFILER_SERVER_RLMSTXCONFIGMANAGER_H
