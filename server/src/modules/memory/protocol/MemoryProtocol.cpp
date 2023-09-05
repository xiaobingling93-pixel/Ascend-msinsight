/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "MemoryProtocol.h"
#include "ProtocolDefs.h"
#include "MemoryProtocolRequest.h"
#include "JsonUtil.h"
#include "MemoryProtocolRespose.h"
#include "MemoryProtocolUtil.h"

namespace Dic {
namespace Protocol {
void MemoryProtocol::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_RES_MEMORY_OPERATOR, ToMemoryOperatorRequest);
    jsonToReqFactory.emplace(REQ_RES_MEMORY_VIEW, ToMemoryViewRequest);
}

void MemoryProtocol::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_RES_MEMORY_OPERATOR, ToMemoryOperatorResponseJson);
    resToJsonFactory.emplace(REQ_RES_MEMORY_VIEW, ToMemoryViewResponseJson);
}

#pragma region <<Json To Request>>

std::unique_ptr<Request> MemoryProtocol::ToMemoryOperatorRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<MemoryOperatorRequest> reqPtr = std::make_unique<MemoryOperatorRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTime, json["params"], "startTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTime, json["params"], "endTime");
    return reqPtr;
}

std::unique_ptr<Request> MemoryProtocol::ToMemoryViewRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<MemoryViewRequest> reqPtr = std::make_unique<MemoryViewRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    return reqPtr;
}

#pragma endregion

#pragma region <<Response To Json>>

std::optional<json_t> MemoryProtocol::ToMemoryOperatorResponseJson(const Response &response)
{
    return ToResponseJson<MemoryOperatorResponse>(dynamic_cast<const MemoryOperatorResponse &>(response));
}

std::optional<json_t> MemoryProtocol::ToMemoryViewResponseJson(const Response &response)
{
    return ToResponseJson<MemoryViewResponse>(dynamic_cast<const MemoryViewResponse &>(response));
}
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic
