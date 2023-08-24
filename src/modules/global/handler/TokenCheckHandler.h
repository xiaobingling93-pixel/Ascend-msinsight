/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SCENE_GLOBAL_TOKEN_CHECK_HANDLER_H
#define DATA_INSIGHT_CORE_SCENE_GLOBAL_TOKEN_CHECK_HANDLER_H

#include "GlobalHandler.h"

namespace Dic {
namespace Module {
class TokenCheckHandler : public GlobalHandler {
public:
    TokenCheckHandler()
    {
        command = REQ_RES_TOKEN_CHECK;
    }
    ~TokenCheckHandler() override = default;
    void HandleRequest(std::unique_ptr<Request> requestPtr) override;
};
} // end of namespace Module
} // Dic

#endif // DATA_INSIGHT_CORE_SCENE_GLOBAL_TOKEN_CHECK_HANDLER_H
