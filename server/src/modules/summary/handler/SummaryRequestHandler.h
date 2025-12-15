/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_SUMMARY_REQUEST_HANDLER_H
#define PROFILER_SERVER_SUMMARY_REQUEST_HANDLER_H

#include "ModuleRequestHandler.h"
#include "ProtocolDefs.h"
#include "SummaryErrorManager.h"

namespace Dic {
namespace Module {
namespace Summary {
class SummaryRequestHandler : public ModuleRequestHandler {
public:
    SummaryRequestHandler()
    {
        moduleName = MODULE_SUMMARY;
        async = true;
    }
    ~SummaryRequestHandler() override = default;
    bool HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override { return true; }
};
} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_SUMMARY_REQUEST_HANDLER_H
