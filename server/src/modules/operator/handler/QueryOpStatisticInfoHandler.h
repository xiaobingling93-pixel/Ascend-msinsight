/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYOPSTATISTICINFOHANDLER_H
#define PROFILER_SERVER_QUERYOPSTATISTICINFOHANDLER_H

#include "ModuleRequestHandler.h"
#include "OperatorRequestHandler.h"
#include "OperatorProtocolRequest.h"

namespace Dic::Module::Operator {
    class QueryOpStatisticInfoHandler : public OperatorRequestHandler {
    public:
        QueryOpStatisticInfoHandler()
        {
            command = REQ_RES_OPERATOR_STATISTIC_INFO;
        }

        ~QueryOpStatisticInfoHandler() override = default;

        void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
    };
}
#endif // PROFILER_SERVER_QUERYOPSTATISTICINFOHANDLER_H
