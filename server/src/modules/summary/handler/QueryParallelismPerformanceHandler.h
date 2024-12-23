/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_HANDLER_QUERYPARALLELISMPERFORMANCEHANDLER_H
#define PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_HANDLER_QUERYPARALLELISMPERFORMANCEHANDLER_H

#include "SummaryProtocolResponse.h"
#include "SummaryRequestHandler.h"
namespace Dic::Module::Summary {
class QueryParallelismPerformanceHandler : public SummaryRequestHandler {
public:
    QueryParallelismPerformanceHandler()
    {
        command = Protocol::REQ_RES_PARALLELISM_PERFORMANCE_DATA;
    }
    ~QueryParallelismPerformanceHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}
#endif // PROFILER_SERVER_SERVER_SRC_MODULES_SUMMARY_HANDLER_QUERYPARALLELISMPERFORMANCEHANDLER_H
