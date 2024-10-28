/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYOPMOREINFOHANDLER_H
#define PROFILER_SERVER_QUERYOPMOREINFOHANDLER_H

#include "ModuleRequestHandler.h"
#include "OperatorRequestHandler.h"
#include "OperatorProtocolRequest.h"

namespace Dic::Module::Operator {
    class QueryOpMoreInfoHandler : public OperatorRequestHandler {
    public:
        QueryOpMoreInfoHandler()
        {
            command = Protocol::REQ_RES_OPERATOR_MORE_INFO;
        }

        ~QueryOpMoreInfoHandler() override = default;

        bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;

        bool CheckRequestParam(OperatorMoreInfoReqParams& params);
    };
}

#endif // PROFILER_SERVER_QUERYOPMOREINFOHANDLER_H
