/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYLEAKSMEMORYALLOCATIONHANDLER_H
#define PROFILER_SERVER_QUERYLEAKSMEMORYALLOCATIONHANDLER_H

#include "MemoryDetailRequestHandler.h"

namespace Dic {
namespace Module {
namespace MemoryDetail {
class QueryLeaksMemoryAllocationHandler : public MemoryDetailRequestHandler {
public:
    QueryLeaksMemoryAllocationHandler() { command = Protocol::REQ_RES_LEAKS_MEMORY_ALLOCATIONS; }
    ~QueryLeaksMemoryAllocationHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;

private:
    void PaddingAllocations(std::vector<MemoryAllocation>& allocations, const LeaksMemoryAllocationParams& queryParams);
};

}  // namespace MemoryDetail
}  // namespace Module
}  // namespace Dic

#endif  // PROFILER_SERVER_QUERYLEAKSMEMORYALLOCATIONHANDLER_H
