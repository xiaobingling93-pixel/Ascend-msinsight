/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYOPERATORDISPATCHHANDLER_H
#define PROFILER_SERVER_QUERYOPERATORDISPATCHHANDLER_H

#include "AdvisorRequestHandler.h"

namespace Dic::Module::Advisor {

class QueryOperatorDispatchHandler : public AdvisorRequestHandler {
public:
    QueryOperatorDispatchHandler()
    {
        command = Protocol::REQ_RES_ADVISOR_OPERATOR_DISPATCH;
    }
    ~QueryOperatorDispatchHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};

} // Dic::Module::Advisor


#endif // PROFILER_SERVER_QUERYOPERATORDISPATCHHANDLER_H
