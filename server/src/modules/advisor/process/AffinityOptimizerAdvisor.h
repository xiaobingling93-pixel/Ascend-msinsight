/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_AFFINITYOPTIMIZER_H
#define PROFILER_SERVER_AFFINITYOPTIMIZER_H

#include <map>
#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"

namespace Dic::Module::Advisor {
const std::map<std::string, std::string> OPTIMIZER_MAP = {
    {"Optimizer.step#SGD.step", "torch_npu.optim.NpuFusedSGD"},
    {"Optimizer.step#Adadelta.step", "torch_npu.optim.NpuFusedAdadelta"},
    {"Optimizer.step#Lamb.step", "torch_npu.optim.NpuFusedLamb"},
    {"Optimizer.step#Adam.step", "torch_npu.optim.NpuFusedAdam"},
    {"Optimizer.step#AdamW.step", "torch_npu.optim.NpuFusedAdamW"},
    {"Optimizer.step#AdamP.step", "torch_npu.optim.NpuFusedAdamP"},
    {"Optimizer.step#BertAdam.step", "torch_npu.optim.NpuFusedBertAdam"},
    {"Optimizer.step#RMSprop.step", "torch_npu.optim.NpuFusedRMSprop"},
    {"Optimizer.step#RMSpropTF.step", "torch_npu.optim.NpuFusedRMSpropTF"}
};

class AffinityOptimizerAdvisor {
public:
static bool Process(const Protocol::APITypeParams& params, Protocol::AffinityOptimizerResBody& resBody);
};
}

#endif // PROFILER_SERVER_AFFINITYOPTIMIZER_H
