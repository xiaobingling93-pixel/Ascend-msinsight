 /*
  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
  */

#ifndef PROFILER_SERVER_QUERYAICPUOPADVICEHANDLER_H
#define PROFILER_SERVER_QUERYAICPUOPADVICEHANDLER_H

#include "AdvisorRequestHandler.h"

namespace Dic {
namespace Module {
namespace Advisor {

class QueryAiCpuOpAdviceHandler : public AdvisorRequestHandler {
public:
    QueryAiCpuOpAdviceHandler()
    {
        command = Protocol::REQ_RES_ADVISOR_AICPU_OPERATORS;
    }
    ~QueryAiCpuOpAdviceHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};

} // Dic
} // Module
} // Advisor

#endif // PROFILER_SERVER_QUERYAICPUOPADVICEHANDLER_H
