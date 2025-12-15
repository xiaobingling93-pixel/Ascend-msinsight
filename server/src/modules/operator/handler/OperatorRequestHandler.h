/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_OPERATORREQUESTHANDLER_H
#define PROFILER_SERVER_OPERATORREQUESTHANDLER_H

#include "ModuleRequestHandler.h"
#include "ProtocolDefs.h"
#include "OperatorErrorManager.h"

namespace Dic::Module::Operator {
    class OperatorRequestHandler : public ModuleRequestHandler {
    public:
        OperatorRequestHandler()
        {
            moduleName = MODULE_OPERATOR;
            async = false;
        }

        ~OperatorRequestHandler() override = default;

        bool HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override { return true; }
    };
}
#endif // PROFILER_SERVER_OPERATORREQUESTHANDLER_H
