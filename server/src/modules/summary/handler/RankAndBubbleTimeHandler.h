/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_RANKANDBUBBLETIME_HANDLER_H
#define PROFILER_SERVER_RANKANDBUBBLETIME_HANDLER_H

#include "SummaryRequestHandler.h"

namespace Dic {
namespace Module {
namespace Summary {
class RankAndBubbleTimeHandler : public SummaryRequestHandler {
public:
    RankAndBubbleTimeHandler()
    {
        command = Protocol::REQ_RES_PIPELINE_RANK_BUBBLE;
    };
    ~RankAndBubbleTimeHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_RANKANDBUBBLETIME_HANDLER_H
