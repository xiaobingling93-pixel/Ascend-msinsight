/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#include "ProtocolDefs.h"
#include "QueryLeaksMemoryPythonTraceHandler.h"
#include "QueryLeaksMemoryDetailHandler.h"
#include "QueryLeaksMemoryBlockHandler.h"
#include "QueryLeaksMemoryAllocationHandler.h"
#include "MemoryDetailModule.h"

namespace Dic {
namespace Module {
MemoryDetailModule::MemoryDetailModule()
{
    moduleName = MODULE_LEAKS;
}

MemoryDetailModule::~MemoryDetailModule()
{
    requestHandlerMap.clear();
}

void MemoryDetailModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_LEAKS_MEMORY_ALLOCATIONS, std::make_unique<QueryLeaksMemoryAllocationHandler>());
    requestHandlerMap.emplace(REQ_RES_LEAKS_MEMORY_BLOCKS, std::make_unique<QueryLeaksMemoryBlockHandler>());
    requestHandlerMap.emplace(REQ_RES_LEAKS_MEMORY_DETAILS, std::make_unique<QueryLeaksMemoryDetailHandler>());
    requestHandlerMap.emplace(REQ_RES_LEAKS_MEMORY_TRACES, std::make_unique<QueryLeaksMemoryPythonTraceHandler>());
}

void MemoryDetailModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
}
}  // end of namespace Module
}  // end of namespace Dic