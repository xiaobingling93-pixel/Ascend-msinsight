/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYCOMPUTEDETAILINFOHANDLER_H
#define PROFILER_SERVER_QUERYCOMPUTEDETAILINFOHANDLER_H

#include "SummaryRequestHandler.h"

namespace Dic {
namespace Module {
namespace Summary {
class QueryComputeDetailInfoHandler : public SummaryRequestHandler {
public:
    QueryComputeDetailInfoHandler()
    {
        command = Protocol::REQ_RES_COMPUTE_DETAIL;
    }
    ~QueryComputeDetailInfoHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};

} // Summary
} // Module
} // Dic

#endif // PROFILER_SERVER_QUERYCOMPUTEDETAILINFOHANDLER_H
