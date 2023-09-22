/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACE_TIME_H
#define PROFILER_SERVER_TRACE_TIME_H

#include <cstdint>
#include <mutex>

namespace Dic {
namespace Module {
namespace Timeline {
class TraceTime {
public:
    static TraceTime &Instance();
    void Reset();
    void UpdateTime(uint64_t min, uint64_t max);
    uint64_t GetStartTime();
    uint64_t GetDuration();

private:
    TraceTime();
    ~TraceTime() = default;
    std::mutex mutex;
    uint64_t maxTimestamp{};
    uint64_t minTimestamp{};
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_TRACE_TIME_H
