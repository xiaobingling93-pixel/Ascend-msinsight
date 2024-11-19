/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "QueryMemoryTypeHandler.h"
#include "QueryMemoryResourceTypeHandler.h"
#include "QueryMemoryOperatorHandler.h"
#include "QueryMemoryComponentHandler.h"
#include "QueryMemoryViewHandler.h"
#include "QueryOperatorSizeHandler.h"
#include "QueryMemoryStaticOperatorGraphHandler.h"
#include "QueryMemoryStaticOperatorListHandler.h"
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
    requestHandlerMap.emplace(REQ_RES_MEMORY_OPERATOR_MIN_MAX, std::make_unique<QueryOperatorSizeHandler>());
    requestHandlerMap.emplace(REQ_RES_MEMORY_STATIC_OP_MEMORY_GRAPH,
                              std::make_unique<QueryMemoryStaticOperatorGraphHandler>());
    requestHandlerMap.emplace(REQ_RES_MEMORY_STATIC_OP_MEMORY_LIST,
                              std::make_unique<QueryMemoryStaticOperatorListHandler>());
}

void MemoryModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
}
} // end of namespace Module
} // end of namespace Dic