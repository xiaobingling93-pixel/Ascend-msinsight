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
    for (const auto &filter : filters) {
        if (!StringUtil::CheckSqlValid(filter.second)) {
            error = "filters exist invalid string value";
            return;
        }
    }
}

bool EventsViewParams::CheckParams(std::string &warnMsg) const
{
    CheckUnsignPageValid(pageSize, currentPage, warnMsg);
    for (const auto &filter : filters) {
        if (!StringUtil::CheckSqlValid(filter.second)) {
            warnMsg = "filters exist invalid string value";
            return false;
        }
    }
    return true;
}
} // namespace Protocol
} // namespace Dic