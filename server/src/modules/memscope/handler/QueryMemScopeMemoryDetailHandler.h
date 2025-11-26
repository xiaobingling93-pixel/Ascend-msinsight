/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_MEM_SCOPE_DETAIL_HANDLER_H
#define PROFILER_SERVER_QUERY_MEM_SCOPE_DETAIL_HANDLER_H

#include "MemScopeRequestHandler.h"

namespace Dic::Module::MemScope {
class QueryMemScopeMemoryDetailHandler : public MemScopeRequestHandler {
public:
    QueryMemScopeMemoryDetailHandler() { command = Protocol::REQ_RES_MEM_SCOPE_MEMORY_DETAILS; }
    ~QueryMemScopeMemoryDetailHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}  // namespace Dic::Module::MemScope
#endif  // PROFILER_SERVER_QUERY_MEM_SCOPE_DETAIL_HANDLER_H