/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "MemoryProtocol.h"
#include "ProtocolDefs.h"
#include "MemoryProtocolRequest.h"
#include "JsonUtil.h"
#include "MemoryProtocolRespose.h"
#include "MemoryProtocolUtil.h"
#include "TimelineProtocol.h"

namespace Dic {
namespace Protocol {
void MemoryProtocol::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_RES_MEMORY_OPERATOR, ToMemoryOperatorRequest);
    jsonToReqFactory.emplace(REQ_RES_MEMORY_VIEW, ToMemoryViewRequest);
    jsonToReqFactory.emplace(REQ_RES_MEMORY_OPERATOR_MIN_MAX, ToMemoryOperatorSizeRequest);
}

void MemoryProtocol::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_RES_MEMORY_OPERATOR, ToMemoryOperatorResponseJson);
    resToJsonFactory.emplace(REQ_RES_MEMORY_VIEW, ToMemoryViewResponseJson);
    resToJsonFactory.emplace(REQ_RES_MEMORY_OPERATOR_MIN_MAX, ToMemoryOperatorSizeResponseJson);
}

void MemoryProtocol::RegisterEventToJsonFuncs()
{
    eventToJsonFactory.emplace(EVENT_MODULE_RESET, TimelineProtocol::ToModuleResetEventJson);
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
    JsonUtil::SetByJsonKeyValue(reqPtr->params.type, json["params"], "type");
    if (json["params"].HasMember("startTime")) {
        JsonUtil::SetByJsonKeyValue(reqPtr->params.startTime, json["params"], "startTime");
    } else {
        reqPtr->params.startTime = -1;
    }
    if (json["params"].HasMember("endTime")) {
        JsonUtil::SetByJsonKeyValue(reqPtr->params.endTime, json["params"], "endTime");
    } else {
        reqPtr->params.endTime = -1;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.currentPage, json["params"], "currentPage");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
    if (json["params"].HasMember("minSize")) {
        JsonUtil::SetByJsonKeyValue(reqPtr->params.minSize, json["params"], "minSize");
    } else {
        reqPtr->params.minSize = -1;
    }
    if (json["params"].HasMember("maxSize")) {
        JsonUtil::SetByJsonKeyValue(reqPtr->params.maxSize, json["params"], "maxSize");
    } else {
        reqPtr->params.maxSize = -1;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.searchName, json["params"], "searchName");
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
    JsonUtil::SetByJsonKeyValue(reqPtr->params.type, json["params"], "type");
    return reqPtr;
}

std::unique_ptr<Request> MemoryProtocol::ToMemoryOperatorSizeRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<MemoryOperatorSizeRequest> reqPtr = std::make_unique<MemoryOperatorSizeRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.type, json["params"], "type");
    return reqPtr;
}

#pragma endregion

#pragma region <<Response To Json>>

std::optional<document_t> MemoryProtocol::ToMemoryOperatorResponseJson(const Response &response)
{
    return ToResponseJson<MemoryOperatorResponse>(dynamic_cast<const MemoryOperatorResponse &>(response));
}

std::optional<document_t> MemoryProtocol::ToMemoryViewResponseJson(const Response &response)
{
    return ToResponseJson<MemoryViewResponse>(dynamic_cast<const MemoryViewResponse &>(response));
}

std::optional<document_t> MemoryProtocol::ToMemoryOperatorSizeResponseJson(const Response &response)
{
    return ToResponseJson<MemoryOperatorSizeResponse>(dynamic_cast<const MemoryOperatorSizeResponse &>(response));
}
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic
