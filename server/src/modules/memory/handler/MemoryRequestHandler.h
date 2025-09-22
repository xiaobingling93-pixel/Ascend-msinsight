/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORY_REQUEST_HANDLER_H
#define PROFILER_SERVER_MEMORY_REQUEST_HANDLER_H

#include "ModuleRequestHandler.h"
#include "ProtocolDefs.h"
#include "NumberUtil.h"

namespace Dic {
namespace Module {
namespace Memory {
class MemoryRequestHandler : public ModuleRequestHandler {
public:
    MemoryRequestHandler()
    {
        moduleName = MODULE_MEMORY;
        async = true;
    }
    ~MemoryRequestHandler() override = default;

    bool HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override { return true; }
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_MEMORY_REQUEST_HANDLER_H
