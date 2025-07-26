/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_RLPROTOCOLREQUEST_H
#define PROFILER_SERVER_RLPROTOCOLREQUEST_H

#include "ProtocolUtil.h"
#include "ProtocolDefs.h"

namespace Dic::Protocol {
struct RLPipelineRequest : public Request {
    RLPipelineRequest() : Request(REQ_REQ_RL_PIPELINE) {};
};
}
#endif // PROFILER_SERVER_RLPROTOCOLREQUEST_H
