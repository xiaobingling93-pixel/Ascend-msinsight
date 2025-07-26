/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#ifndef PROFILER_SERVER_LEAKSMEMORYPYTHONTRACE_H
#define PROFILER_SERVER_LEAKSMEMORYPYTHONTRACE_H

#include "pch.h"

namespace Dic {
namespace Module {
namespace MemoryDetail {
struct PythonTraceSlice {
    std::string func;
    uint64_t startTimestamp;
    uint64_t endTimestamp;
    int depth;

    PythonTraceSlice() : func("UNKNOWN"), startTimestamp(0), endTimestamp(0), depth(-1) {}
    PythonTraceSlice(std::string func, uint64_t startTimestamp, uint64_t endTimestamp, int depth)
        : func(std::move(func)),
          startTimestamp(startTimestamp),
          endTimestamp(endTimestamp),
          depth(depth) {}
};
struct LeaksMemoryPythonTrace {
    uint64_t maxTimestamp;
    uint64_t minTimestamp;
    uint64_t threadId;
    std::vector<PythonTraceSlice> slices;

    LeaksMemoryPythonTrace() : maxTimestamp(0), minTimestamp(INT64_MAX), threadId(0) {}

    [[nodiscard]] bool Empty() const
    {
        return slices.empty();
    }
};
} // Memory
} // Module
} // Dic

#endif // PROFILER_SERVER_LEAKSMEMORYPYTHONTRACE_H
