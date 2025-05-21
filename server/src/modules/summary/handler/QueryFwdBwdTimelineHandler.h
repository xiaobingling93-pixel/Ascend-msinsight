/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYFWDBWDTIMELINEHANDLER_H
#define PROFILER_SERVER_QUERYFWDBWDTIMELINEHANDLER_H

#include <map>
#include "SummaryProtocolResponse.h"
#include "SummaryRequestHandler.h"
namespace Dic::Module::Summary {

class QueryFwdBwdTimelineHandler : public SummaryRequestHandler {
public:
    QueryFwdBwdTimelineHandler()
    {
        command = Protocol::REQ_RES_PIPELINE_FWD_BWD_TIMELINE;
    }
    ~QueryFwdBwdTimelineHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
private:
    static bool QueryFwdBwdTimelineByRank(const std::string &rankId, const std::string &stepId);
    static std::map<std::string, PipelineFwdBwdTimelineByRank> dataMap;
};

} // Dic::Module::Summary

#endif // PROFILER_SERVER_QUERYFWDBWDTIMELINEHANDLER_H
