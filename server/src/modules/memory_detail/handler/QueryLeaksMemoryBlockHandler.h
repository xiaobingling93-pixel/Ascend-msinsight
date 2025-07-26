/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYLEAKSMEMORYBLOCKHANDLER_H
#define PROFILER_SERVER_QUERYLEAKSMEMORYBLOCKHANDLER_H

#include "MemoryDetailRequestHandler.h"

namespace Dic {
namespace Module {
namespace MemoryDetail {
class QueryLeaksMemoryBlockHandler : public MemoryDetailRequestHandler {
public:
    QueryLeaksMemoryBlockHandler() { command = Protocol::REQ_RES_LEAKS_MEMORY_BLOCKS; }
    ~QueryLeaksMemoryBlockHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
    static void BuildBlocksResponse(const std::vector<MemoryBlock>& blocks, LeaksMemoryBlocksResponse& response);
};
}  // namespace MemoryDetail
}  // namespace Module
}  // namespace Dic

#endif  // PROFILER_SERVER_QUERYLEAKSMEMORYBLOCKHANDLER_H
