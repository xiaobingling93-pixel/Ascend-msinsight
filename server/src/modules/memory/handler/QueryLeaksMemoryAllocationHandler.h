/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYLEAKSMEMORYALLOCATIONHANDLER_H
#define PROFILER_SERVER_QUERYLEAKSMEMORYALLOCATIONHANDLER_H

#include "MemoryProtocolRequest.h"
#include "MemoryProtocolRespose.h"
#include "LeaksMemoryDatabase.h"
#include "MemoryRequestHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
class QueryLeaksMemoryAllocationHandler : public MemoryRequestHandler {
public:
    QueryLeaksMemoryAllocationHandler()
    {
        command = Protocol::REQ_RES_LEAKS_MEMORY_ALLOCATIONS;
    }
    ~QueryLeaksMemoryAllocationHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
private:
    void PaddingAllocations(std::vector<MemoryAllocation> &allocations, const LeaksMemoryAllocationParams &queryParams);
};

} // Memory
} // Module
} // Dic

#endif // PROFILER_SERVER_QUERYLEAKSMEMORYALLOCATIONHANDLER_H
