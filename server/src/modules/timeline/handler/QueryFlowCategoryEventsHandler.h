/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_FLOW_CATEGORY_EVENTS_HANDLER_H
#define PROFILER_SERVER_QUERY_FLOW_CATEGORY_EVENTS_HANDLER_H

#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class QueryFlowCategoryEventsHandler : public TimelineRequestHandler {
public:
    QueryFlowCategoryEventsHandler()
    {
        command = Protocol::REQ_RES_FLOW_CATEGORY_EVENTS;
    };
    ~QueryFlowCategoryEventsHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_QUERY_FLOW_CATEGORY_EVENTS_HANDLER_H
