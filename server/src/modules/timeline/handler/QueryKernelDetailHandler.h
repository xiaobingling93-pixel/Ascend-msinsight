/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/

#ifndef PROFILER_SERVER_QUERYKERNELDETAILHANDLER_H
#define PROFILER_SERVER_QUERYKERNELDETAILHANDLER_H

#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class QueryKernelDetailHandler : public TimelineRequestHandler {
public:
    QueryKernelDetailHandler()
    {
        command = Protocol::REQ_RES_UNIT_KERNEL_DETAILS;
    };

    ~QueryKernelDetailHandler() override = default;

    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_QUERYKERNELDETAILHANDLER_H
