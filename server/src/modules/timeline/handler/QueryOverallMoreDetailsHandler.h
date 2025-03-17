/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYOVERALLMOREDETAILS_H
#define PROFILER_SERVER_QUERYOVERALLMOREDETAILS_H
#include "TimelineRequestHandler.h"
#include "DataBaseManager.h"
#include "SystemViewOverallHelper.h"

namespace Dic::Module::Timeline {
class QueryOverallMoreDetailsHandler : public TimelineRequestHandler {
public:
    QueryOverallMoreDetailsHandler()
    {
        command = Protocol::REQ_RES_SYSTEM_VIEW_OVERALL_MORE_DETAILS;
    };

    ~QueryOverallMoreDetailsHandler() override = default;

    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
private:
    static void GetComputingOverallMetricDetails(const std::shared_ptr<VirtualTraceDatabase> &database,
        const SystemViewOverallReqParam &params, UnitThreadsOperatorsResponse &response,
        uint64_t minTimestamp, SystemViewOverallHelper &overallHelper);
    static void GetPagedData(std::vector<SameOperatorsDetails>& filteredEvents,
        const SystemViewOverallReqParam &params, UnitThreadsOperatorsResponse &response);
    static void GetCommunicationOverallMetricDetails(const std::shared_ptr<VirtualTraceDatabase> &database,
        const SystemViewOverallReqParam &params, UnitThreadsOperatorsResponse &response,
        uint64_t minTimestamp, SystemViewOverallHelper &overallHelper);
};
}
#endif // PROFILER_SERVER_QUERYOVERALLMOREDETAILS_H