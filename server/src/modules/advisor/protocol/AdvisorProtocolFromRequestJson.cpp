/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "AdvisorProtocolRequest.h"
#include "JsonUtil.h"
#include "ProtocolUtil.h"
#include "AdvisorProtocolFromRequestJson.h"

namespace Dic::Protocol {
std::unique_ptr<Request> AdvisorProtocolFromRequestJson::ToAffinityOptimizerRequest(
    const Dic::json_t &json, std::string &error)
{
    std::unique_ptr<AffinityOptimizerRequest> reqPtr = std::make_unique<AffinityOptimizerRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }

    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    return reqPtr;
}

std::unique_ptr<Request> AdvisorProtocolFromRequestJson::ToAffinityAPIRequest(
    const Dic::json_t &json, std::string &error)
{
    return {};
}

std::unique_ptr<Request> AdvisorProtocolFromRequestJson::ToOperatorFusionRequest(
    const json_t &json, std::string &error)
{
    return {};
}

std::unique_ptr<Request> AdvisorProtocolFromRequestJson::ToAICpuOperatorRequest(
    const json_t &json, std::string &error)
{
    return {};
}

std::unique_ptr<Request> AdvisorProtocolFromRequestJson::ToAclnnOperatorRequest(
    const json_t &json, std::string &error)
{
    return {};
}
}
