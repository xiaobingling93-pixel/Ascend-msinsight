/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SCENE_BASE_SCENE_H
#define DATA_INSIGHT_CORE_SCENE_BASE_SCENE_H

#include <memory>
#include "GlobalDefs.h"
#include "Protocol.h"
#include "ModuleRequestHandler.h"

namespace Dic {
namespace Module {
using namespace Dic::Protocol;
class BaseModule {
public:
    BaseModule() = default;
    virtual ~BaseModule() = default;

    virtual void RegisterRequestHandlers() = 0;
    virtual void OnRequest(std::unique_ptr<Protocol::Request> request);

protected:
    ModuleType sceneType = ModuleType::UNKNOWN;
    std::map<std::string, std::unique_ptr<ModuleRequestHandler>> requestHandlerMap;
};
} // end of namespace Module
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SCENE_BASE_SCENE_H
