/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "RLProtocol.h"
#include "ProtocolDefs.h"
#include "RLProtocolRequest.h"
#include "RLProtocolResponse.h"
#include "RLProtocolUtil.h"

namespace Dic::Protocol {
void RLProtocol::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_REQ_RL_PIPELINE, ToRLPipelineRequest);
}

void RLProtocol::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_REQ_RL_PIPELINE, ToRLPipelineResponse);
}

void RLProtocol::RegisterEventToJsonFuncs()
{
}

#pragma region <<Json To Request>>
std::unique_ptr<Request> RLProtocol::ToRLPipelineRequest(const json_t &json, std::string &error)
{
    auto reqPtr = std::make_unique<RLPipelineRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    return reqPtr;
}

#pragma endregion

#pragma region <<Reponse To Json>>

std::optional<document_t> RLProtocol::ToRLPipelineResponse(const Response &response)
{
    return ToResponseJson<RLPipelineResponse>(dynamic_cast<const RLPipelineResponse &>(response));
}

#pragma endregion
}
