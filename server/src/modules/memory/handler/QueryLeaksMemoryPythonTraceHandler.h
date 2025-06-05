/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */


#ifndef PROFILER_SERVER_QUERYLEAKSMEMORYPYTHONTRACEHANDLER_H
#define PROFILER_SERVER_QUERYLEAKSMEMORYPYTHONTRACEHANDLER_H

#include "pch.h"
#include "MemoryRequestHandler.h"
#include "MemoryProtocolRequest.h"
#include "MemoryProtocolRespose.h"
#include "LeaksMemoryService.h"
#include "LeaksMemoryDetailTreeNode.h"


namespace Dic {
namespace Module {
namespace Memory {
class QueryLeaksMemoryPythonTraceHandler : public MemoryRequestHandler {
public:
    QueryLeaksMemoryPythonTraceHandler()
    {
        command = Protocol::REQ_RES_LEAKS_MEMORY_TRACES;
    }
    ~QueryLeaksMemoryPythonTraceHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // Memory
} // Module
} // Dic

#endif // PROFILER_SERVER_QUERYLEAKSMEMORYPYTHONTRACEHANDLER_H
