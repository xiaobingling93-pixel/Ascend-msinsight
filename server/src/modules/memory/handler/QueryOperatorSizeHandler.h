/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_OPERATOR_SIZE_HANDLER_H
#define PROFILER_SERVER_QUERY_OPERATOR_SIZE_HANDLER_H

#include "MemoryRequestHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
class QueryOperatorSizeHandler : public MemoryRequestHandler {
public:
    QueryOperatorSizeHandler()
    {
        command = Protocol::REQ_RES_MEMORY_OPERATOR_MIN_MAX;
    };
    ~QueryOperatorSizeHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_QUERY_OPERATOR_SIZE_HANDLER_H
