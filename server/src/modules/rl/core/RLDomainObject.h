/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_RLDOMAINOBJECT_H
#define PROFILER_SERVER_RLDOMAINOBJECT_H
#include <string>
#include <vector>
#include <unordered_map>

namespace Dic::Module::RL {
    struct MicroBatchConfig {
        std::string batchName;
        std::string type;
    };

    struct TaskConfig {
        std::string roleName;
        std::string taskName;
        std::vector<MicroBatchConfig> microBatchConfigs;
        std::unordered_map<std::string, MicroBatchConfig> microBatchConfigMap;
        void AddMicroBatchConf(MicroBatchConfig&& config)
        {
            microBatchConfigs.push_back(config);
            microBatchConfigMap[config.batchName] = config;
        }
    };

    struct RLMstxConfig {
        std::string framework;
        std::string algorithm;
        std::vector<TaskConfig> taskConfigs;
        std::unordered_map<std::string, TaskConfig> taskConfigMap;
        void AddTaskConfig(TaskConfig&& config)
        {
            taskConfigs.push_back(config);
            taskConfigMap[config.taskName] = config;
        }
    };
}
#endif // PROFILER_SERVER_RLDOMAINOBJECT_H