/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYAFFINITYAPIADVICE_H
#define PROFILER_SERVER_QUERYAFFINITYAPIADVICE_H

#include "AdvisorRequestHandler.h"

namespace Dic::Module::Advisor {

class QueryAffinityAPIAdvice : public AdvisorRequestHandler {
public:
    QueryAffinityAPIAdvice()
    {
        command = Protocol::REQ_RES_ADVISOR_AFFINITY_API;
    }
    ~QueryAffinityAPIAdvice() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};

} // Dic::Module::Advisor

#endif // PROFILER_SERVER_QUERYAFFINITYAPIADVICE_H
