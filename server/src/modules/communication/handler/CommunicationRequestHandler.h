/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATION_REQUEST_HANDLER_H
#define PROFILER_SERVER_COMMUNICATION_REQUEST_HANDLER_H

#include "ModuleRequestHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
class CommunicationRequestHandler : public ModuleRequestHandler {
public:
    CommunicationRequestHandler()
    {
        moduleName = MODULE_COMMUNICATION;
        async = false;
    }
    ~CommunicationRequestHandler() override = default;
    bool HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override { return true; }
};
} // end of namespace Communication
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_COMMUNICATION_REQUEST_HANDLER_H
