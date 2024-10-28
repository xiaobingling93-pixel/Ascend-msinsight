//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#ifndef PROFILER_SERVER_QUERY_THREAD_TRACES_HANDLER_H
#define PROFILER_SERVER_QUERY_THREAD_TRACES_HANDLER_H

#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class QueryThreadTracesHandler : public TimelineRequestHandler {
public:
    QueryThreadTracesHandler()
    {
        command = Protocol::REQ_RES_UNIT_THREAD_TRACES;
    };
    ~QueryThreadTracesHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic


#endif // PROFILER_SERVER_QUERY_THREAD_TRACES_HANDLER_H
