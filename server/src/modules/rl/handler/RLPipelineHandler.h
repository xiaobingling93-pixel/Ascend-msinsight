/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_RLPIPELINEHANDLER_H
#define PROFILER_SERVER_RLPIPELINEHANDLER_H

#include "RLRequestHandler.h"

namespace Dic::Module::RL {
class RLPipelineHandler : public RLRequestHandler {
public:
    RLPipelineHandler()
    {
        command = Protocol::REQ_REQ_RL_PIPELINE;
    }
    ~RLPipelineHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}
#endif // PROFILER_SERVER_RLPIPELINEHANDLER_H
