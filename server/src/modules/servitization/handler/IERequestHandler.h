/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_IEREQUESTHANDLER_H
#define PROFILER_SERVER_IEREQUESTHANDLER_H
#include "ProtocolDefs.h"
#include "ModuleRequestHandler.h"
namespace Dic {
namespace Module {
namespace IE {
class IERequestHandler : public ModuleRequestHandler {
public:
    IERequestHandler()
    {
        moduleName = MODULE_IE;
    }

    ~IERequestHandler() override = default;
    bool HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override
    {
        return true;
    };
};
}
}
}
#endif // PROFILER_SERVER_IEREQUESTHANDLER_H
