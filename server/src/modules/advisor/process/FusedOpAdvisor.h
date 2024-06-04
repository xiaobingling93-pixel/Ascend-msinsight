/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_FUSEDOPADVISOR_H
#define PROFILER_SERVER_FUSEDOPADVISOR_H

#include "VirtualTraceDatabase.h"
#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"

namespace Dic::Module::Advisor {
const std::vector<Timeline::FuseableOpRule> FUSEABLE_OPERATER_RULE_LIST = {
    {{"Cast", "LayerNorm", "Cast"}, "LayerNorm", ""},
    {{"Transpose", "Transpose", "GatherElement", "Transpose"}, "GatherElements", ""},
    {{"Mul", "Slice", "Neg", "Slice", "ConcatD", "Mul", "Add"}, "RotaryMul", ""},
    {{"Mul", "AsStrided", "Neg", "AsStrided", "ConcatD", "Mul", "Add"}, "RotaryMul", ""},
    {{"Mul", "Slice", "Neg", "Slice", "ConcatD", "Cast", "Mul", "Add"}, "RotaryMul", ""},
    {{"Cast", "Square", "MemSet", "ReduceMean", "Add", "Rsqrt", "Mul", "Cast", "Mul"}, "RMSNorm", ""},
    {
        {"Cast", "Square", "ReduceMeanD", "Add", "Rsqrt", "Cast", "Cast", "Mul", "Cast", "Cast", "Mul", "Cast"},
        "RMSNORM",
        ""
    },
};

const std::vector<std::string> FUSED_OP_ORDER_BY_NAME_LIST = {
    "startTime", "duration", "pid", "tid", "name"
};
class FusedOpAdvisor {
public:
    static bool Process(const Protocol::APITypeParams& params, Protocol::OperatorFusionResBody& resBody);
};

} // Dic::Module::Advisor

#endif // PROFILER_SERVER_FUSEDOPADVISOR_H
