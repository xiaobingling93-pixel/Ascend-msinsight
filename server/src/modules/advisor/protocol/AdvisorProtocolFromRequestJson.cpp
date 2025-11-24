/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "AdvisorProtocolRequest.h"
#include "ProtocolMessage.h"
#include "AdvisorProtocolFromRequestJson.h"

namespace Dic::Protocol {
template<typename RequestType> std::unique_ptr<Request> ToRequest(const Dic::json_t& json, std::string& error)
{
    std::unique_ptr<RequestType> reqPtr = std::make_unique<RequestType>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.currentPage, json["params"], "current");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderType, json["params"], "order");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTime, json["params"], "startTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTime, json["params"], "endTime");
    return reqPtr;
}

std::unique_ptr<Request> AdvisorProtocolFromRequestJson::ToAffinityOptimizerRequest(
    const Dic::json_t &json, std::string &error)
{
    return ToRequest<AffinityOptimizerRequest>(json, error);
}

std::unique_ptr<Request> AdvisorProtocolFromRequestJson::ToAffinityAPIRequest(
    const Dic::json_t &json, std::string &error)
{
    return ToRequest<AffinityAPIRequest>(json, error);
}

std::unique_ptr<Request> AdvisorProtocolFromRequestJson::ToOperatorFusionRequest(
    const json_t &json, std::string &error)
{
    return ToRequest<OperatorFusionRequest>(json, error);
}

std::unique_ptr<Request> AdvisorProtocolFromRequestJson::ToAICpuOperatorRequest(
    const json_t &json, std::string &error)
{
    return ToRequest<AICpuOperatorRequest>(json, error);
}

std::unique_ptr<Request> AdvisorProtocolFromRequestJson::ToAclnnOperatorRequest(
    const json_t &json, std::string &error)
{
    return ToRequest<AclnnOperatorRequest>(json, error);
}

std::unique_ptr<Request> AdvisorProtocolFromRequestJson::ToOperatorDispatchRequest(
    const json_t &json, std::string &error)
{
    return ToRequest<OperatorDispatchRequest>(json, error);
}
}
