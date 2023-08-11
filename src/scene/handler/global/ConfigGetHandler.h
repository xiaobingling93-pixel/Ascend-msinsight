/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SCENE_GLOBAL_CONFIG_GET_HANDLER_H
#define DATA_INSIGHT_CORE_SCENE_GLOBAL_CONFIG_GET_HANDLER_H

#include "GlobalHandler.h"

namespace Dic {
namespace Scene {
class ConfigGetHandler : public GlobalHandler {
public:
    ConfigGetHandler()
    {
        command = REQ_RES_CONFIG_GET;
    }
    ~ConfigGetHandler() override = default;
    void HandleRequest(std::unique_ptr<Request> requestPtr) override;
};
} // end of namespace Scene
} // Dic

#endif // DATA_INSIGHT_CORE_SCENE_GLOBAL_CONFIG_GET_HANDLER_H
