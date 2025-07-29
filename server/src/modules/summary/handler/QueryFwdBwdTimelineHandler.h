/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYFWDBWDTIMELINEHANDLER_H
#define PROFILER_SERVER_QUERYFWDBWDTIMELINEHANDLER_H

#include <map>
#include "SummaryProtocolResponse.h"
#include "SummaryRequestHandler.h"
#include "VirtualTraceDatabase.h"

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
    static bool QueryFwdBwdTimelineByRank(const std::string &rankId, const std::string &stepId,
                                          const std::string &clusterPath);
    static std::map<std::string, PipelineFwdBwdTimelineByRank> dataMap;
    static bool QueryFwdBwdTimelineFromFlow(const std::string &rankId, const std::string &stepId,
        const std::shared_ptr<Dic::Module::Timeline::VirtualTraceDatabase> &database);
    static bool QueryFwdBwdTimelineFromMstx(const std::string &rankId, const std::string &stepId,
        const std::shared_ptr<Dic::Module::Timeline::VirtualTraceDatabase> &database);
    static void CalFlowInfo(std::vector<FlowInfo> &flowList, const std::vector<std::string> &rankIds);
};

} // Dic::Module::Summary

#endif // PROFILER_SERVER_QUERYFWDBWDTIMELINEHANDLER_H
