//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#ifndef PROFILER_SERVER_TIMELINE_REQUEST_HANDLER_H
#define PROFILER_SERVER_TIMELINE_REQUEST_HANDLER_H
#include <utility>

#include "RenderEngineInterface.h"
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
        moduleName = MODULE_TIMELINE;
    }
    ~TimelineRequestHandler() override = default;
    bool HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr) override{ return true; };
    void SetRenderEngine(std::shared_ptr<RenderEngineInterface> renderEngineInterface)
    {
        renderEngine = std::move(renderEngineInterface);
    };

protected:
    std::shared_ptr<RenderEngineInterface> renderEngine = nullptr;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_TIMELINE_REQUEST_HANDLER_H
