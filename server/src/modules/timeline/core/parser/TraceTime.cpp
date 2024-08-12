/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "TraceTime.h"

namespace Dic {
namespace Module {
namespace Timeline {

TraceTime &TraceTime::Instance()
{
    static TraceTime instance;
    return instance;
}

void TraceTime::Reset()
{
    std::unique_lock<std::mutex> lock(mutex);
    maxTimestamp = 0;
    minTimestamp = UINT64_MAX;
}

TraceTime::TraceTime()
{
    Reset();
}

void TraceTime::UpdateTime(uint64_t min, uint64_t max)
{
    std::unique_lock<std::mutex> lock(mutex);
    minTimestamp = std::min(minTimestamp, min);
    maxTimestamp = std::max(maxTimestamp, max);
}

uint64_t TraceTime::GetStartTime()
{
    std::unique_lock<std::mutex> lock(mutex);
    return minTimestamp;
}

uint64_t TraceTime::GetDuration()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (maxTimestamp >= minTimestamp) {
        return maxTimestamp - minTimestamp;
    } else {
        Server::ServerLog::Warn("Max timestamp is less than min timestamp. Max timestamp:", maxTimestamp,
                                ", min timestamp:", minTimestamp);
        return 0;
    }
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic