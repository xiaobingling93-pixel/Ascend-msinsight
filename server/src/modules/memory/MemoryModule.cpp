/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "QueryMemoryTypeHandler.h"
#include "QueryMemoryResourceTypeHandler.h"
#include "QueryMemoryOperatorHandler.h"
#include "QueryMemoryComponentHandler.h"
#include "QueryMemoryViewHandler.h"
#include "QueryMemoryOperatorSizeHandler.h"
#include "QueryMemoryStaticOperatorGraphHandler.h"
#include "QueryMemoryStaticOperatorListHandler.h"
#include "QueryMemoryStaticOperatorSizeHandler.h"
#include "QueryLeaksMemoryAllocationHandler.h"
#include "QueryLeaksMemoryBlockHandler.h"
#include "QueryLeaksMemoryDetailHandler.h"
#include "QueryLeaksMemoryPythonTraceHandler.h"
#include "FindSliceByAllocationTimeHandler.h"
#include "RepositoryFactory.h"
#include "DataEngine.h"
#include "RenderEngine.h"
#include "ProtocolDefs.h"
#include "MemoryModule.h"

namespace Dic {
namespace Module {
using namespace Memory;
MemoryModule::MemoryModule()
{
    moduleName = MODULE_MEMORY;
}

MemoryModule::~MemoryModule()
{
    requestHandlerMap.clear();
}

void MemoryModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_MEMORY_TYPE, std::make_unique<QueryMemoryTypeHandler>());
    requestHandlerMap.emplace(REQ_RES_MEMORY_RESOURCE_TYPE, std::make_unique<QueryMemoryResourceTypeHandler>());
    requestHandlerMap.emplace(REQ_RES_MEMORY_OPERATOR, std::make_unique<QueryMemoryOperatorHandler>());
    requestHandlerMap.emplace(REQ_RES_MEMORY_COMPONENT, std::make_unique<QueryMemoryComponentHandler>());
    requestHandlerMap.emplace(REQ_RES_MEMORY_VIEW, std::make_unique<QueryMemoryViewHandler>());
    requestHandlerMap.emplace(REQ_RES_MEMORY_OPERATOR_MIN_MAX, std::make_unique<QueryMemoryOperatorSizeHandler>());
    requestHandlerMap.emplace(REQ_RES_MEMORY_STATIC_OP_MEMORY_GRAPH,
        std::make_unique<QueryMemoryStaticOperatorGraphHandler>());
    requestHandlerMap.emplace(REQ_RES_MEMORY_STATIC_OP_MEMORY_LIST,
        std::make_unique<QueryMemoryStaticOperatorListHandler>());
    requestHandlerMap.emplace(REQ_RES_MEMORY_STATIC_OP_MEMORY_MIN_MAX,
        std::make_unique<QueryMemoryStaticOperatorSizeHandler>());
    requestHandlerMap.emplace(REQ_RES_LEAKS_MEMORY_ALLOCATIONS,
                              std::make_unique<QueryLeaksMemoryAllocationHandler>());
    requestHandlerMap.emplace(REQ_RES_LEAKS_MEMORY_BLOCKS,
                              std::make_unique<QueryLeaksMemoryBlockHandler>());
    requestHandlerMap.emplace(REQ_RES_LEAKS_MEMORY_DETAILS,
                              std::make_unique<QueryLeaksMemoryDetailHandler>());
    requestHandlerMap.emplace(REQ_RES_LEAKS_MEMORY_TRACES,
                              std::make_unique<QueryLeaksMemoryPythonTraceHandler>());
    auto renderEngine = Timeline::RenderEngine::Instance();
    auto findSliceByAllocationTimeHandler = std::make_unique<FindSliceByAllocationTimeHandler>(renderEngine);
    requestHandlerMap.emplace(REQ_RES_MEMORY_FIND_SLICE, std::move(findSliceByAllocationTimeHandler));
}

void MemoryModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
}
} // end of namespace Module
} // end of namespace Dic