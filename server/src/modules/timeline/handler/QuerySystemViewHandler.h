/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/

#ifndef PROFILER_SERVER_QUERYSYSTEMVIEWHANDLER_H
#define PROFILER_SERVER_QUERYSYSTEMVIEWHANDLER_H

#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class QuerySystemViewHandler : public TimelineRequestHandler {
public:
    QuerySystemViewHandler()
    {
        command = Protocol::REQ_RES_UNIT_SYSTEM_VIEW;
    };

    ~QuerySystemViewHandler() override = default;

    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_QUERYSYSTEMVIEWHANDLER_H
