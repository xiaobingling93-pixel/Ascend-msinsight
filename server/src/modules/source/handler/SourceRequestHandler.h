/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SOURCE_REQUEST_HANDLER_H
#define PROFILER_SERVER_SOURCE_REQUEST_HANDLER_H

#include "ModuleRequestHandler.h"

namespace Dic {
namespace Module {
namespace Source {
class SourceRequestHandler : public ModuleRequestHandler {
public:
    SourceRequestHandler()
    {
        moduleName = Protocol::ModuleType::SOURCE;
        async = false;
    }
    ~SourceRequestHandler() override = default;
    void HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override {}
};
} // end of namespace Source
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_SOURCE_REQUEST_HANDLER_H
