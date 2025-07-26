/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_RLREQUESTHANDLER_H
#define PROFILER_SERVER_RLREQUESTHANDLER_H

#include "ModuleRequestHandler.h"
#include "ProtocolDefs.h"

namespace Dic::Module::RL {
class RLRequestHandler : public ModuleRequestHandler {
public:
    RLRequestHandler()
    {
        moduleName = MODULE_RL;
        async = false;
    }
    ~RLRequestHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override { return true; }
};
}
#endif // PROFILER_SERVER_RLREQUESTHANDLER_H
