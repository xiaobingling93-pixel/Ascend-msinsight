/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SCENE_MANAGER_H
#define DATA_INSIGHT_CORE_SCENE_MANAGER_H

#include <memory>
#include <mutex>
#include "Protocol.h"
#include "BaseModule.h"

namespace Dic {
namespace Module {
class ModuleManager {
public:
    static ModuleManager &Instance();
    bool SetGlobalConfig(const GlobalConfig &config);
    bool SetAscendConfig(const TimelineConfig &config);
    const std::optional<GlobalConfig> GetGlobalConfig();
    const std::optional<TimelineConfig> GetAscendConfig();
    void OnDispatchSceneRequest(std::unique_ptr<Request> request);

private:
    ModuleManager();
    ~ModuleManager();

    void Register();
    void UnRegister();

    std::mutex mutex;
    std::map<Dic::Protocol::ModuleType, std::unique_ptr<BaseModule>> sceneMap;
};
} // end of namespace Module
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SCENE_MANAGER_H
