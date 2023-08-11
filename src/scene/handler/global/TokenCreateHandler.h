/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SCENE_GLOBAL_TOKEN_CREATE_HANDLER_H
#define DATA_INSIGHT_CORE_SCENE_GLOBAL_TOKEN_CREATE_HANDLER_H

#include "GlobalHandler.h"

namespace Dic {
namespace Scene {
class TokenCreateHandler : public GlobalHandler {
public:
    TokenCreateHandler()
    {
        command = REQ_RES_TOKEN_CREATE;
    }
    ~TokenCreateHandler() override = default;
    void HandleRequest(std::unique_ptr<Request> requestPtr) override;
};
} // end of namespace Scene
} // Dic

#endif // DATA_INSIGHT_CORE_SCENE_GLOBAL_TOKEN_CREATE_HANDLER_H
