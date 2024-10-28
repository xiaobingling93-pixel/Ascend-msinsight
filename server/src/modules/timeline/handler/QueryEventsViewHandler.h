/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_EVENTS_VIEW_HANDLER_H
#define PROFILER_SERVER_QUERY_EVENTS_VIEW_HANDLER_H

#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class QueryEventsViewHandler : public TimelineRequestHandler {
public:
    QueryEventsViewHandler()
    {
        command = Protocol::REQ_RES_UNIT_EVENTS_VIEW;
    };
    ~QueryEventsViewHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_QUERY_EVENTS_VIEW_HANDLER_H
