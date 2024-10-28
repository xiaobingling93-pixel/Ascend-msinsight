/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYAFFINITYOPTIMIZERADVICE_H
#define PROFILER_SERVER_QUERYAFFINITYOPTIMIZERADVICE_H

#include "AdvisorRequestHandler.h"

namespace Dic::Module::Advisor {

class QueryAffinityOptimizerAdvice : public AdvisorRequestHandler {
public:
    QueryAffinityOptimizerAdvice()
    {
        command = Protocol::REQ_RES_ADVISOR_AFFINITY_OPTIMIZER;
    }
    ~QueryAffinityOptimizerAdvice() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // Dic::Module::Advisor

#endif // PROFILER_SERVER_QUERYAFFINITYOPTIMIZERADVICE_H
