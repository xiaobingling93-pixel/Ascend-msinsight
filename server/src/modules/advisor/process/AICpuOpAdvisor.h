 /*
  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
  */

#ifndef PROFILER_SERVER_AICPUOPADVISOR_H
#define PROFILER_SERVER_AICPUOPADVISOR_H

#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"

namespace Dic::Module::Advisor {
const uint64_t AICPU_OP_DURATION_THRESHOLD = 20000; // 20us
class AICpuOpAdvisor {
public:
    static bool Process(const Protocol::APITypeParams& params, Protocol::AICpuOperatorResBody& resBody);
};

} // Dic::Module::Advisor

#endif // PROFILER_SERVER_AICPUOPADVISOR_H
