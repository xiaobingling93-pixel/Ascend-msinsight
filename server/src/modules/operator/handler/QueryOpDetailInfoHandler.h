/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYOPDETAILINFOHANDLER_H
#define PROFILER_SERVER_QUERYOPDETAILINFOHANDLER_H

#include "ModuleRequestHandler.h"
#include "OperatorRequestHandler.h"
#include "OperatorProtocolRequest.h"

namespace Dic::Module::Operator {
    class QueryOpDetailInfoHandler : public OperatorRequestHandler {
    public:
        QueryOpDetailInfoHandler()
        {
            command = REQ_RES_OPERATOR_DETAIL_INFO;
        }

        ~QueryOpDetailInfoHandler() override = default;

        void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
    };
}

#endif // PROFILER_SERVER_QUERYOPDETAILINFOHANDLER_H
