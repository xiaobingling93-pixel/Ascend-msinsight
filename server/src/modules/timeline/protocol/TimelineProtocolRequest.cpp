/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "TimelineProtocolRequest.h"

namespace Dic {
namespace Protocol {
void KernelDetailsParams::Check(std::string &error) const
{
    if (current == 0) {
        error = "current is invalid";
        return;
    }
    if (pageSize == 0) {
        error = "pageSize is invalid";
        return;
    }
}

bool EventsViewParams::CheckParams(std::string &warnMsg) const
{
    CheckUnsignPageValid(pageSize, currentPage, warnMsg);
    for (const auto &filter : filters) {
        if (!StringUtil::CheckSqlValid(filter.first)) {
            warnMsg = "filters exist invalid string value";
            return false;
        }
    }
    return true;
}

bool UnitThreadsParams::CheckParams(uint64_t minTime, std::string &warnMsg) const
{
    if (startTime > endTime) {
        warnMsg = "unit threads start time is bigger than end time";
        return false;
    }
    if (endTime > UINT64_MAX - minTime) {
        warnMsg = "unit threads end time is invalid";
        return false;
    }
    if (!CheckStrParamValidEmptyAllowed(startDepth, warnMsg)) {
        warnMsg = "unit threads start depth is invalid";
        return false;
    }
    if (!CheckStrParamValidEmptyAllowed(endDepth, warnMsg)) {
        warnMsg = "unit threads end depth is invalid";
        return false;
    }
    if (!startDepth.empty() && !endDepth.empty()) {
        if (NumberUtil::StringToUint32(startDepth) > NumberUtil::StringToUint32(endDepth)) {
            warnMsg = "unit threads start depth is bigger than end depth";
            return false;
        }
    }
    return true;
}
} // namespace Protocol
} // namespace Dic