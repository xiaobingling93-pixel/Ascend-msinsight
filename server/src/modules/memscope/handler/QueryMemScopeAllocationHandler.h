/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_MEM_SCOPE_ALLOCATION_HANDLER_H
#define PROFILER_SERVER_QUERY_MEM_SCOPE_ALLOCATION_HANDLER_H

#include "MemScopeRequestHandler.h"

namespace Dic {
namespace Module {
namespace MemScope {
class QueryMemScopeAllocationHandler : public MemScopeRequestHandler {
public:
    QueryMemScopeAllocationHandler() { command = Protocol::REQ_RES_MEM_SCOPE_MEMORY_ALLOCATIONS; }
    ~QueryMemScopeAllocationHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;

private:
    void PaddingAllocations(std::vector<MemoryAllocation>& allocations,
                            const MemScopeMemoryAllocationParams& queryParams);
};

}  // namespace MemScope
}  // namespace Module
}  // namespace Dic

#endif  // PROFILER_SERVER_QUERY_MEM_SCOPE_ALLOCATION_HANDLER_H
