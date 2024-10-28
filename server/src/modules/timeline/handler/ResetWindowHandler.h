//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#ifndef PROFILER_SERVER_RESET_WINDOW_HANDLER_H
#define PROFILER_SERVER_RESET_WINDOW_HANDLER_H

#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class ResetWindowHandler : public TimelineRequestHandler {
public:
    ResetWindowHandler()
    {
        command = Protocol::REQ_RES_RESET_WINDOW;
        async = false;
    };
    ~ResetWindowHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_RESET_WINDOW_HANDLER_H
