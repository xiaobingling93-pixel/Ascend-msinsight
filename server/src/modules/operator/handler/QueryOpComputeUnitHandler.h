/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYOPCOMPUTEUNITHANDLER_H
#define PROFILER_SERVER_QUERYOPCOMPUTEUNITHANDLER_H

#include "ModuleRequestHandler.h"
#include "OperatorRequestHandler.h"
#include "OperatorProtocolRequest.h"

namespace Dic::Module::Operator {
    class QueryOpComputeUnitHandler : public OperatorRequestHandler {
    public:
        QueryOpComputeUnitHandler()
        {
            command = Protocol::REQ_RES_OPERATOR_COMPUTE_UNIT_INFO;
        }

        ~QueryOpComputeUnitHandler() override = default;

        bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;

    private:
        bool CheckRequestParam(OperatorDurationReqParams params);
    };
}

#endif // PROFILER_SERVER_QUERYOPCOMPUTEUNITHANDLER_H
