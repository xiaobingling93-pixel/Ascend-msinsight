/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SCENE_HARMONY_SCENE_H
#define DATA_INSIGHT_CORE_SCENE_HARMONY_SCENE_H

#include "BaseScene.h"

namespace Dic {
namespace Scene {
class HarmonyScene : public BaseScene {
public:
    HarmonyScene();
    ~HarmonyScene() override;

    void Config(const HarmonyConfig &cfg);
    const HarmonyConfig &GetConfig() const;
    void RegisterRequestHandlers() override;
    void OnRequest(std::unique_ptr<Protocol::Request> request) override;

private:
    HarmonyConfig config;
    void RegisterHdcRequestHandlers();
    void RegisterDfxRequestHandlers();
};
} // end of namespace Scene
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SCENE_HARMONY_SCENE_H
