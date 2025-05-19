/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_OPERATORDISPATCHADVISOR_H
#define PROFILER_SERVER_OPERATORDISPATCHADVISOR_H

#include "VirtualTraceDatabase.h"
#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"


namespace Dic::Module::Advisor {

const uint64_t OPERATOR_COMPILE_CNT_THRESHOLD = 20;
const std::vector<std::string> OPERATOR_DISPATCH_ORDER_BY_NAME_LIST = {
    "startTime", "duration", "pid", "tid", "name"
};
const std::string SUGGESTION_NOTE = "Please use `torch_npu.npu.set_compile_mode(jit_compile=False)` "
                                    "to disable jit compile in dynamic shape usage.";

class OperatorDispatchAdvisor {
public:
    static bool Process(const Protocol::APITypeParams& params, Protocol::OperatorDispatchResBody& resBody);
};

} // Dic::Module::Advisor


#endif // PROFILER_SERVER_OPERATORDISPATCHADVISOR_H
