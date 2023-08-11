/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SCENE_HARMONY_HANDLER_H
#define DATA_INSIGHT_CORE_SCENE_HARMONY_HANDLER_H

#include "SceneRequestHandler.h"

namespace Dic {
namespace Scene {
class HarmonyHandler : public SceneRequestHandler {
public:
    HarmonyHandler()
    {
        sceneType = SceneType::HARMONY;
    }
    ~HarmonyHandler() override = default;
    void HandleRequest(std::unique_ptr<Request> requestPtr) override {};

protected:
    std::string scope;
};
} // end of namespace Scene
} // Dic

#endif // DATA_INSIGHT_CORE_SCENE_HARMONY_HANDLER_H