/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYLEAKSMEMORYDETAILHANDLER_H
#define PROFILER_SERVER_QUERYLEAKSMEMORYDETAILHANDLER_H

#include "MemoryRequestHandler.h"
#include "MemoryProtocolRequest.h"
#include "MemoryProtocolRespose.h"
#include "LeaksMemoryService.h"
#include "LeaksMemoryDetailTreeNode.h"

namespace Dic {
namespace Module {
namespace Memory {
class QueryLeaksMemoryDetailHandler : public MemoryRequestHandler {
public:
    QueryLeaksMemoryDetailHandler()
    {
        command = Protocol::REQ_RES_LEAKS_MEMORY_DETAILS;
    }
    ~QueryLeaksMemoryDetailHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // Memory
} // Module
} // Dic


#endif // PROFILER_SERVER_QUERYLEAKSMEMORYDETAILHANDLER_H
