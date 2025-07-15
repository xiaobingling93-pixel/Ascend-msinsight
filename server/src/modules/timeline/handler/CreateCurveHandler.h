/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_CREATECURVEHANDLER_H
#define PROFILER_SERVER_CREATECURVEHANDLER_H
#include "ServitizationOpenApi.h"
#include "TimelineRequestHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
class CreateCurveHandler : public TimelineRequestHandler {
public:
    std::shared_ptr<IE::ServitizationOpenApi> openApi = std::make_shared<IE::ServitizationOpenApi>();
    CreateCurveHandler()
    {
        command = Protocol::REQ_RES_CREATE_CURVE;
    };
    ~CreateCurveHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;

    std::string CreateSliceDurationCurve(const CreateCurveRequest &request);

    std::string CreateBubbleCurve(const CreateCurveRequest &request);
};
}  // end of namespace Timeline
}  // end of namespace Module
}  // end of namespace Dic
#endif  // PROFILER_SERVER_CREATECURVEHANDLER_H
