/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SCENE_BASE_SCENE_H
#define DATA_INSIGHT_CORE_SCENE_BASE_SCENE_H

#include <memory>
#include "GlobalDefs.h"
#include "Protocol.h"
#include "SceneRequestHandler.h"

namespace Dic {
namespace Scene {
using namespace Dic::Protocol;
class BaseScene {
public:
    BaseScene() = default;
    virtual ~BaseScene() = default;

    virtual void RegisterRequestHandlers() = 0;
    virtual void OnRequest(std::unique_ptr<Protocol::Request> request);

protected:
    SceneType sceneType = SceneType::UNKNOWN;
    std::map<std::string, std::unique_ptr<SceneRequestHandler>> requestHandlerMap;
};
} // end of namespace Scene
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SCENE_BASE_SCENE_H
