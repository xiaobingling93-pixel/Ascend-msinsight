/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "RLPipelineHandler.h"
#include "RLProtocolRequest.h"
#include "RLProtocolResponse.h"
#include "RLPipelineService.h"

namespace Dic::Module::RL {
bool RLPipelineHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<Protocol::RLPipelineRequest &>(*requestPtr);
    std::unique_ptr<Protocol::RLPipelineResponse> responsePtr = std::make_unique<Protocol::RLPipelineResponse>();
    RLPipelineResponse &response = *responsePtr;
    bool res = RLPipelineService::Instance().GetPipelineInfo(response);
    SetBaseResponse(request, response);
    SendResponse(std::move(responsePtr), res);
    return true;
}
}