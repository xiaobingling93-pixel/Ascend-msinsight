/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "MemoryModule.h"
#include "QueryMemoryOperatorHandler.h"
#include "QueryMemoryViewHandler.h"

namespace Dic {
namespace Module {
using namespace Memory;
MemoryModule::MemoryModule()
{
    moduleName = ModuleType::MEMORY;
}

MemoryModule::~MemoryModule()
{
    requestHandlerMap.clear();
}

void MemoryModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_MEMORY_OPERATOR, std::make_unique<QueryMemoryOperatorHandler>());
    requestHandlerMap.emplace(REQ_RES_MEMORY_VIEW, std::make_unique<QueryMemoryViewHandler>());
}

void MemoryModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
}
} // end of namespace Module
} // end of namespace Dic