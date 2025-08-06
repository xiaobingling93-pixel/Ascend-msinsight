/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#ifndef PROFILER_SERVER_MEMORY_DETAIL_ENTITIES_H
#define PROFILER_SERVER_MEMORY_DETAIL_ENTITIES_H

#include "MemoryDetailDefs.h"

namespace Dic::Module::MemoryDetail {
// [PYTHON_TRACE] 内存调用栈火焰图相关实体
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
    uint64_t maxTimestamp{};
    uint64_t minTimestamp{INT64_MAX};
    uint64_t threadId{};
    std::vector<PythonTraceSlice> slices;
    int maxDepth{};

    [[nodiscard]] bool Empty() const;
};
// [PYTHON_TRACE]

// [EVENT] 事件扩展属性attr(json)实体
struct BlockEventAttr {
    std::string addr;
    int64_t size{0};
    std::string owner;
    uint64_t total{0};
    uint64_t used{0};
    int64_t mid{0};
};
// [EVENT]
} // Dic::Module::MemoryDetail

#endif  // PROFILER_SERVER_MEMORY_DETAIL_ENTITIES_H
