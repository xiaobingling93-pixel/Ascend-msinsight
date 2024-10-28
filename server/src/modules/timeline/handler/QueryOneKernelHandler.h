/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/

#ifndef PROFILER_SERVER_QUERYONEKERNELHANDLER_H
#define PROFILER_SERVER_QUERYONEKERNELHANDLER_H

#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class QueryOneKernelHandler : public TimelineRequestHandler {
public:
    QueryOneKernelHandler()
    {
        command = Protocol::REQ_RES_ONE_KERNEL_DETAILS;
    };

    ~QueryOneKernelHandler() override = default;

    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_QUERYONEKERNELHANDLER_H
