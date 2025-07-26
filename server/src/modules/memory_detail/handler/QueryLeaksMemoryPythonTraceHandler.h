/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYLEAKSMEMORYPYTHONTRACEHANDLER_H
#define PROFILER_SERVER_QUERYLEAKSMEMORYPYTHONTRACEHANDLER_H

#include "MemoryDetailRequestHandler.h"

namespace Dic {
namespace Module {
namespace MemoryDetail {
class QueryLeaksMemoryPythonTraceHandler : public MemoryDetailRequestHandler {
public:
    QueryLeaksMemoryPythonTraceHandler() { command = Protocol::REQ_RES_LEAKS_MEMORY_TRACES; }
    ~QueryLeaksMemoryPythonTraceHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}  // namespace MemoryDetail
}  // namespace Module
}  // namespace Dic

#endif  // PROFILER_SERVER_QUERYLEAKSMEMORYPYTHONTRACEHANDLER_H
