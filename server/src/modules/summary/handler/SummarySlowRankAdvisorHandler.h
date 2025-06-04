/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_SUMMARYSLOWRANKADVISORHANDLER_H
#define PROFILER_SERVER_SUMMARYSLOWRANKADVISORHANDLER_H

#include "SummaryProtocolResponse.h"
#include "SummaryRequestHandler.h"
namespace Dic::Module::Summary {
class SummarySlowRankAdvisorHandler : public SummaryRequestHandler {
public:
    SummarySlowRankAdvisorHandler()
    {
        command = Protocol::REQ_RES_SUMMARY_SLOW_RANK_ADVISOR;
    }
    ~SummarySlowRankAdvisorHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}


#endif // PROFILER_SERVER_SUMMARYSLOWRANKADVISORHANDLER_H
