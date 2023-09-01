//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#ifndef PROFILER_SERVER_TIMELINE_REQUEST_HANDLER_H
#define PROFILER_SERVER_TIMELINE_REQUEST_HANDLER_H

#include "ModuleRequestHandler.h"
#include "TimelineProtocolRequest.h"
#include "TimelineProtocolResponse.h"

namespace Dic {
namespace Module {
namespace Timeline {
class TimelineRequestHandler : public ModuleRequestHandler {
public:
    TimelineRequestHandler()
    {
        moduleName = Protocol::ModuleType::TIMELINE;
    }
    ~TimelineRequestHandler() override = default;
    void HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override {}
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_TIMELINE_REQUEST_HANDLER_H
