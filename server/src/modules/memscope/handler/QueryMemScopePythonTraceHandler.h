/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_MEM_SCOPE_PYTHON_TRACE_HANDLER_H
#define PROFILER_SERVER_QUERY_MEM_SCOPE_PYTHON_TRACE_HANDLER_H

#include "MemScopeRequestHandler.h"

namespace Dic {
namespace Module {
namespace MemScope {
class QueryMemScopePythonTraceHandler : public MemScopeRequestHandler {
public:
    QueryMemScopePythonTraceHandler() { command = Protocol::REQ_RES_MEM_SCOPE_PYTHON_TRACES; }
    ~QueryMemScopePythonTraceHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;

private:
    const size_t TRIM_THRESHOLD = 20000;
    const PythonTrimCompressStrategy DEFAULT_TRIM_STRATEGY = PythonTrimCompressStrategy::COMPRESS_SMALL_FUNCTIONS;
};
}  // namespace MemScope
}  // namespace Module
}  // namespace Dic

#endif  // PROFILER_SERVER_QUERY_MEM_SCOPE_PYTHON_TRACE_HANDLER_H
