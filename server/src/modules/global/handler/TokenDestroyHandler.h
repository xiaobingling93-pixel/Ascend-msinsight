/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_MODULE_GLOBAL_TOKEN_DESTROY_HANDLER_H
#define DATA_INSIGHT_CORE_MODULE_GLOBAL_TOKEN_DESTROY_HANDLER_H

#include "GlobalHandler.h"
#include "GlobalProtocolRequest.h"

namespace Dic {
namespace Module {
class TokenDestroyHandler : public GlobalHandler {
public:
    TokenDestroyHandler()
    {
        command = REQ_RES_TOKEN_DESTROY;
    }
    ~TokenDestroyHandler() override = default;
    void HandleRequest(std::unique_ptr<Request> requestPtr) override;

private:
    void OnDestroySession(const TokenDestroyRequest &request) const;
};
} // end of namespace Module
} // Dic

#endif // DATA_INSIGHT_CORE_MODULE_GLOBAL_TOKEN_DESTROY_HANDLER_H
