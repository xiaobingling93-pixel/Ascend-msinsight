/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYOPCATEGORYINFOHANDLER_H
#define PROFILER_SERVER_QUERYOPCATEGORYINFOHANDLER_H

#include "ModuleRequestHandler.h"
#include "OperatorRequestHandler.h"
#include "OperatorProtocolRequest.h"

namespace Dic::Module::Operator {
    class QueryOpCategoryInfoHandler : public OperatorRequestHandler {
    public:
        QueryOpCategoryInfoHandler()
        {
            command = Protocol::REQ_RES_OPERATOR_CATEGORY_INFO;
        }

        ~QueryOpCategoryInfoHandler() override = default;

        bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;

    private:
        bool CheckRequestParam(OperatorDurationReqParams params);
    };
}

#endif // PROFILER_SERVER_QUERYOPCATEGORYINFOHANDLER_H
