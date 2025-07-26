/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "RLPipelineHandler.h"
#include "RLProtocolRequest.h"
#include "RLProtocolResponse.h"
#include "pch.h"

namespace Dic::Module::RL {
using namespace Dic;
using namespace Dic::Server;
bool RLPipelineHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<Protocol::RLPipelineRequest &>(*requestPtr);
    std::unique_ptr<Protocol::RLPipelineResponse> responsePtr = std::make_unique<Protocol::RLPipelineResponse>();
    RLPipelineResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    SendResponse(std::move(responsePtr), true);
    return true;
}
}