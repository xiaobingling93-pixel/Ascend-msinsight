/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "CommunicationProtocolUtil.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "CommunicationProtocol.h"

namespace Dic {
namespace Protocol {
void CommunicationProtocol::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_RES_COMMUNICATION_OPERATOR_DETAILS, ToOperatorDetailsRequest);
    jsonToReqFactory.emplace(REQ_RES_COMMUNICATION_DISTRIBUTION, ToDistributionRequest);
    jsonToReqFactory.emplace(REQ_RES_COMMUNICATION_BANDWIDTH, ToBandwidthDataRequest);
}

void CommunicationProtocol::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_RES_COMMUNICATION_OPERATOR_DETAILS, ToOperatorDetailsResponse);
    resToJsonFactory.emplace(REQ_RES_COMMUNICATION_DISTRIBUTION, ToDistributionResponse);
    resToJsonFactory.emplace(REQ_RES_COMMUNICATION_BANDWIDTH, ToBandwidthDataResponse);
}

void CommunicationProtocol::RegisterEventToJsonFuncs()
{
}

#pragma region <<Json To Request>>

std::unique_ptr<Request> CommunicationProtocol::ToOperatorDetailsRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<OperatorDetailsRequest> reqPtr = std::make_unique<OperatorDetailsRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.iterationId, json["params"], "iterationId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.stage, json["params"], "stage");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.currentPage, json["params"], "currentPage");
    return reqPtr;
}

std::unique_ptr<Request> CommunicationProtocol::ToDistributionRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<DistributionDataRequest> reqPtr = std::make_unique<DistributionDataRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.dbIndex, json["params"], "dbIndex");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.iterationId, json["params"], "iterationId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.operatorName, json["params"], "operatorName");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.transportType, json["params"], "transportType");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.stage, json["params"], "stage");
    return reqPtr;
}

std::unique_ptr<Request> CommunicationProtocol::ToBandwidthDataRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<BandwidthDataRequest> reqPtr = std::make_unique<BandwidthDataRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.dbIndex, json["params"], "dbIndex");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.iterationId, json["params"], "iterationId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.operatorName, json["params"], "operatorName");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.stage, json["params"], "stage");
    return reqPtr;
}

#pragma endregion

#pragma region <<Json To Request>>

std::optional<json_t> CommunicationProtocol::ToOperatorDetailsResponse(const Response &response)
{
    return ToResponseJson<OperatorDetailsResponse>(dynamic_cast<const OperatorDetailsResponse &>(response));
}

std::optional<json_t> CommunicationProtocol::ToBandwidthDataResponse(const Response &response)
{
    return ToResponseJson<BandwidthDataResponse>(dynamic_cast<const BandwidthDataResponse &>(response));
}

std::optional<json_t> CommunicationProtocol::ToDistributionResponse(const Response &response)
{
    return ToResponseJson<DistributionResponse>(dynamic_cast<const DistributionResponse &>(response));
}

#pragma endregion
} // namespace Protocol
} // namespace Dic
