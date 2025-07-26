/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#ifndef PROFILER_SERVER_MEMORY_DETAIL_REQUEST_HANDLER_H
#define PROFILER_SERVER_MEMORY_DETAIL_REQUEST_HANDLER_H

#include "pch.h"
#include "ModuleRequestHandler.h"
#include "ProtocolDefs.h"
#include "NumberUtil.h"
#include "MemoryDetailProtocolRequest.h"
#include "MemoryDetailProtocolResponse.h"
#include "MemoryDetailProtocol.h"
#include "LeaksMemoryDatabase.h"
#include "LeaksMemoryService.h"
#include "LeaksMemoryDetailTreeNode.h"

namespace Dic::Module::MemoryDetail {
class MemoryDetailRequestHandler : public ModuleRequestHandler {
public:
    MemoryDetailRequestHandler()
    {
        moduleName = MODULE_LEAKS;
        async = false;
    }
    ~MemoryDetailRequestHandler() override = default;
};
} // namespace Dic::Module::MemoryDetail
#endif  // PROFILER_SERVER_MEMORY_DETAIL_REQUEST_HANDLER_H
