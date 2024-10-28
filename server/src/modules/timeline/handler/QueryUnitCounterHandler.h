/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERY_UNIT_COUNTER_HANDLER_H
#define PROFILER_SERVER_QUERY_UNIT_COUNTER_HANDLER_H

#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class QueryUnitCounterHandler : public TimelineRequestHandler {
public:
    QueryUnitCounterHandler()
    {
        command = Protocol::REQ_RES_UNIT_COUNTER;
    };
    ~QueryUnitCounterHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_QUERY_UNIT_COUNTER_HANDLER_H
