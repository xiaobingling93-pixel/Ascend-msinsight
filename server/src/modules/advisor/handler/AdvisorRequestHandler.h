/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_ADVISORREQUESTHANDLER_H
#define PROFILER_SERVER_ADVISORREQUESTHANDLER_H

#include "ModuleRequestHandler.h"

namespace Dic::Module::Advisor {
class AdvisorRequestHandler : public ModuleRequestHandler {
public:
    AdvisorRequestHandler()
    {
        moduleName = MODULE_ADVISOR;
        async = false;
    }
    ~AdvisorRequestHandler() override = default;
    bool HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override { return true; }
};
} // Advisor

#endif // PROFILER_SERVER_ADVISORREQUESTHANDLER_H
