/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <math.h>
#include "TraceTime.h"

namespace Dic {
namespace Scene {
namespace Core {

TraceTime &TraceTime::Instance()
{
    static TraceTime instance;
    return instance;
}

void TraceTime::Reset()
{
    maxTimestamp = 0;
    minTimestamp = UINT64_MAX;
}

TraceTime::TraceTime()
{
    Reset();
}

void TraceTime::UpdateTime(uint64_t min, uint64_t max)
{
    minTimestamp = std::min(minTimestamp, min);
    maxTimestamp = std::max(maxTimestamp, max);
}

uint64_t TraceTime::GetStartTime()
{
    return minTimestamp;
}

uint64_t TraceTime::GetDuration()
{
    return maxTimestamp - minTimestamp;
}
} // end of namespace Core
} // end of namespace Scene
} // end of namespace Dic