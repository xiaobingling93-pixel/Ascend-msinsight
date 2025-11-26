/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#ifndef PROFILER_SERVER_MEM_SCOPE_REQUEST_HANDLER_H
#define PROFILER_SERVER_MEM_SCOPE_REQUEST_HANDLER_H

#include "pch.h"
#include "ModuleRequestHandler.h"
#include "ProtocolDefs.h"
#include "NumberUtil.h"
#include "MemScopeProtocolRequest.h"
#include "MemScopeProtocolResponse.h"
#include "MemScopeProtocol.h"
#include "MemScopeDatabase.h"
#include "MemScopeService.h"
#include "MemScopeEventTree.h"

namespace Dic::Module::MemScope {
class MemScopeRequestHandler : public ModuleRequestHandler {
public:
    MemScopeRequestHandler()
    {
        moduleName = MODULE_MEM_SCOPE;
        async = false;
    }
    ~MemScopeRequestHandler() override = default;
};
} // namespace Dic::Module::MemScope
#endif  // PROFILER_SERVER_MEM_SCOPE_REQUEST_HANDLER_H
