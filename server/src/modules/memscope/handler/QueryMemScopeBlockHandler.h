/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_MEM_SCOPE_BLOCK_HANDLER_H
#define PROFILER_SERVER_QUERY_MEM_SCOPE_BLOCK_HANDLER_H

#include "MemScopeRequestHandler.h"

namespace Dic {
namespace Module {
namespace MemScope {
class QueryMemScopeBlockHandler : public MemScopeRequestHandler {
public:
    QueryMemScopeBlockHandler() { command = Protocol::REQ_RES_MEM_SCOPE_MEMORY_BLOCKS; }
    ~QueryMemScopeBlockHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
    static void BuildBlocksViewResponse(const std::vector<MemoryBlock>& blocks, MemScopeMemoryBlocksResponse& response);

    static bool HandleBlocksTableRequest(MemScopeMemoryBlockRequest& request,
                                         MemScopeMemoryBlocksResponse& response,
                                         std::string &errorMsg);
    static bool HandleBlocksViewRequest(MemScopeMemoryBlockRequest& request,
                                        MemScopeMemoryBlocksResponse& response,
                                        std::string &errorMsg);
};
}  // namespace MemScope
}  // namespace Module
}  // namespace Dic

#endif  // PROFILER_SERVER_QUERY_MEM_SCOPE_BLOCK_HANDLER_H
