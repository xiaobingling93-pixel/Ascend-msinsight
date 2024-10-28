 /*
  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
  */

#ifndef PROFILER_SERVER_QUERYFUSEDOPADVICEHANDLER_H
#define PROFILER_SERVER_QUERYFUSEDOPADVICEHANDLER_H

#include "AdvisorRequestHandler.h"

namespace Dic::Module::Advisor {

class QueryFusedOpAdviceHandler : public AdvisorRequestHandler {
public:
    QueryFusedOpAdviceHandler()
    {
        command = Protocol::REQ_RES_ADVISOR_OPERATORS_FUSION;
    }
    ~QueryFusedOpAdviceHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};

} // Dic::Module::Advisor

#endif // PROFILER_SERVER_QUERYFUSEDOPADVICEHANDLER_H
