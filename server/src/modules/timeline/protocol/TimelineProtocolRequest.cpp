/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "TimelineProtocolRequest.h"

namespace Dic {
namespace Protocol {
void KernelDetailsParams::Check(std::string &error) const
{
    if (current == 0) {
        error = "current is invaild";
        return;
    }
    if (pageSize == 0) {
        error = "pageSize is invaild";
        return;
    }
    for (const auto &filter : filters) {
        if (!StringUtil::CheckSqlValid(filter.second)) {
            error = "filters exist invalid string value";
            return;
        }
    }
}
} // namespace Protocol
} // namespace Dic