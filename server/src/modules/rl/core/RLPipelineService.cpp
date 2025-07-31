/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "RLPipelineService.h"
#include "DataBaseManager.h"
#include "RenderEngine.h"
#include "NumberSafeUtil.h"
#include "TrackInfoManager.h"
#include "DataBaseManager.h"

namespace Dic::Module::RL {
std::vector<Protocol::RLPipelineNode> RLPipelineService::SearchNode(const std::string &fileId)
{
    return {};
}

bool RLPipelineService::GetPipelineInfo(Protocol::RLPipelineResponse &response)
{
    return true;
}

std::vector<Protocol::RLPipelineNode> RLPipelineService::QueryMicroBatch(const std::string &fileId,
    const RLMstxConfig &config, const RLPipelineNode &node)
{
    return std::vector<Protocol::RLPipelineNode>();
}
}