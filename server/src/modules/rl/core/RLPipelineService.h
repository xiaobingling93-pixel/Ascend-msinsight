/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_RLPIPELINESERVICE_H
#define PROFILER_SERVER_RLPIPELINESERVICE_H

#include <vector>
#include "pch.h"
#include "RLProtocolResponse.h"
#include "VirtualTraceDatabase.h"
#include "DomainObject.h"
#include "RLDomainObject.h"

namespace Dic::Module::RL {
class RLPipelineService {
public:
    bool GetPipelineInfo(Protocol::RLPipelineResponse &response);
private:
    std::vector<Protocol::RLPipelineNode> SearchNode(const std::string &fileId);
    std::vector<Protocol::RLPipelineNode> QueryMicroBatch(const std::string &fileId, const RLMstxConfig &config,
                                                          const Protocol::RLPipelineNode &node);
    uint64_t minTime = UINT64_MAX;
    uint64_t maxTime = 0;
};
}
#endif // PROFILER_SERVER_RLPIPELINESERVICE_H
