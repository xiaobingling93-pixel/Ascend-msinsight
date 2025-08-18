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

private:
    const size_t TRIM_THRESHOLD = 20000;
    const PythonTrimCompressStrategy DEFAULT_TRIM_STRATEGY = PythonTrimCompressStrategy::COMPRESS_SMALL_FUNCTIONS;
};
}  // namespace MemoryDetail
}  // namespace Module
}  // namespace Dic

#endif  // PROFILER_SERVER_QUERYLEAKSMEMORYPYTHONTRACEHANDLER_H
