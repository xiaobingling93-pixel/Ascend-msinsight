/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_FUSEDOPADVISOR_H
#define PROFILER_SERVER_FUSEDOPADVISOR_H

#include "AdvisorProtocolRequest.h"
#include "AdvisorProtocolResponse.h"

namespace Dic::Module::Advisor {

class FusedOpAdvisor {
public:
    static bool Process(const Protocol::APITypeParams& params, Protocol::OperatorFusionResBody& resBody);
};

} // Dic::Module::Advisor

#endif // PROFILER_SERVER_FUSEDOPADVISOR_H
