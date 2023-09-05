/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_MEMORY_OPERATOR_HANDLER_H
#define PROFILER_SERVER_QUERY_MEMORY_OPERATOR_HANDLER_H

#include "MemoryRequestHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
class QueryMemoryOperatorHandler : public MemoryRequestHandler {
public:
    QueryMemoryOperatorHandler()
    {
        command = Protocol::REQ_RES_MEMORY_OPERATOR;
    };
    ~QueryMemoryOperatorHandler() override = default;
    void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic


#endif // PROFILER_SERVER_QUERY_MEMORY_OPERATOR_HANDLER_H
