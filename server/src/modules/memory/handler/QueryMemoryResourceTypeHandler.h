// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

#ifndef PROFILER_SERVER_QUERY_MEMORY_RESOURCE_TYPE_HANDLER_H
#define PROFILER_SERVER_QUERY_MEMORY_RESOURCE_TYPE_HANDLER_H


#include "MemoryRequestHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
class QueryMemoryResourceTypeHandler : public MemoryRequestHandler {
public:
    QueryMemoryResourceTypeHandler()
    {
        command = Protocol::REQ_RES_MEMORY_RESOURCE_TYPE;
    };
    ~QueryMemoryResourceTypeHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_QUERY_MEMORY_RESOURCE_TYPE_HANDLER_H
