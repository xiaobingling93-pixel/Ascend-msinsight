/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SCENE_MANAGER_H
#define DATA_INSIGHT_CORE_SCENE_MANAGER_H

#include <memory>
#include <mutex>
#include "Protocol.h"
#include "BaseScene.h"

namespace Dic {
namespace Scene {
class SceneManager {
public:
    static SceneManager &Instance();
    bool SetGlobalConfig(const GlobalConfig &config);
    bool SetHarmonyConfig(const HarmonyConfig &config);
    const std::optional<GlobalConfig> GetGlobalConfig();
    const std::optional<HarmonyConfig> GetHarmonyConfig();
    void OnDispatchSceneRequest(std::unique_ptr<Request> request);

private:
    SceneManager();
    ~SceneManager();

    void Register();
    void UnRegister();

    std::mutex mutex;
    std::map<Dic::Protocol::SceneType, std::unique_ptr<BaseScene>> sceneMap;
};
} // end of namespace Scene
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SCENE_MANAGER_H
