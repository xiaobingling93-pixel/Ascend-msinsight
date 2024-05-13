/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */


#ifndef PROFILER_SERVER_ADVISORPROTOCOLUTIL_H
#define PROFILER_SERVER_ADVISORPROTOCOLUTIL_H

#include "ProtocolUtil.h"

namespace Dic::Protocol {
class AdvisorProtocolUtil : public ProtocolUtil {
public:
    AdvisorProtocolUtil() = default;
    ~AdvisorProtocolUtil() override = default;

private:
    void RegisterJsonToRequestFuncs() override;
    void RegisterResponseToJsonFuncs() override;
    void RegisterEventToJsonFuncs() override;
};
}

#endif // PROFILER_SERVER_ADVISORPROTOCOLUTIL_H
