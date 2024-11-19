/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_MEMORY_COMPONENT_HANDLER_H
#define PROFILER_SERVER_QUERY_MEMORY_COMPONENT_HANDLER_H

#include "MemoryRequestHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
class QueryMemoryComponentHandler : public MemoryRequestHandler {
public:
    QueryMemoryComponentHandler()
    {
        command = Protocol::REQ_RES_MEMORY_COMPONENT;
    }
    ~QueryMemoryComponentHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_QUERY_MEMORY_COMPONENT_HANDLER_H
