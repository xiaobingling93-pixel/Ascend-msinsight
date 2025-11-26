/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#include "ProtocolDefs.h"
#include "QueryMemScopePythonTraceHandler.h"
#include "QueryMemScopeMemoryDetailHandler.h"
#include "QueryMemScopeBlockHandler.h"
#include "QueryMemScopeAllocationHandler.h"
#include "QueryMemScopeEventHandler.h"
#include "MemScopeModule.h"

using namespace Dic::Module::MemScope;

namespace Dic {
namespace Module {
MemScopeModule::MemScopeModule()
{
    moduleName = MODULE_MEM_SCOPE;
}

MemScopeModule::~MemScopeModule()
{
    requestHandlerMap.clear();
}

void MemScopeModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_MEM_SCOPE_MEMORY_ALLOCATIONS, std::make_unique<QueryMemScopeAllocationHandler>());
    requestHandlerMap.emplace(REQ_RES_MEM_SCOPE_MEMORY_BLOCKS, std::make_unique<QueryMemScopeBlockHandler>());
    requestHandlerMap.emplace(REQ_RES_MEM_SCOPE_MEMORY_DETAILS, std::make_unique<QueryMemScopeMemoryDetailHandler>());
    requestHandlerMap.emplace(REQ_RES_MEM_SCOPE_PYTHON_TRACES, std::make_unique<QueryMemScopePythonTraceHandler>());
    requestHandlerMap.emplace(REQ_RES_MEM_SCOPE_EVENTS, std::make_unique<QueryMemScopeEventHandler>());
}

void MemScopeModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
}
}  // end of namespace Module
}  // end of namespace Dic