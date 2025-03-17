/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYSYSTEMVIEWOVERALLHANDLER_H
#define PROFILER_SERVER_QUERYSYSTEMVIEWOVERALLHANDLER_H
#include "TimelineRequestHandler.h"
#include "DataBaseManager.h"
#include "SystemViewOverallHelper.h"

namespace Dic {
namespace Module {
namespace Timeline {
class QuerySystemViewOverallHandler : public TimelineRequestHandler {
public:
    QuerySystemViewOverallHandler()
    {
        command = Protocol::REQ_RES_SYSTEM_VIEW_OVERALL;
    };

    ~QuerySystemViewOverallHandler() override = default;

    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
private:
    static double GetOverlapAnalysisData(SystemViewOverallHelper &overallHelper,
        const std::shared_ptr<VirtualTraceDatabase> &database, const SystemViewOverallRequest &request,
        std::vector<SystemViewOverallRes> &responseBody);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_QUERYSYSTEMVIEWOVERALLHANDLER_H