/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "pch.h"
#include "ProtocolDefs.h"
#include "MemoryProtocolRequest.h"
#include "MemoryProtocolRespose.h"
#include "MemoryProtocolUtil.h"
#include "TimelineProtocol.h"
#include "MemoryProtocol.h"

namespace Dic {
namespace Protocol {
void MemoryProtocol::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_RES_MEMORY_TYPE, ToMemoryTypeRequest);
    jsonToReqFactory.emplace(REQ_RES_MEMORY_RESOURCE_TYPE, ToMemoryResourceTypeRequest);
    jsonToReqFactory.emplace(REQ_RES_MEMORY_OPERATOR, ToMemoryOperatorRequest);
    jsonToReqFactory.emplace(REQ_RES_MEMORY_COMPONENT, ToMemoryComponentRequest);
    jsonToReqFactory.emplace(REQ_RES_MEMORY_FIND_SLICE, ToMemoryFindSliceRequest);
    jsonToReqFactory.emplace(REQ_RES_MEMORY_VIEW, ToMemoryViewRequest);
    jsonToReqFactory.emplace(REQ_RES_MEMORY_OPERATOR_MIN_MAX, ToMemoryOperatorSizeRequest);
    jsonToReqFactory.emplace(REQ_RES_MEMORY_STATIC_OP_MEMORY_GRAPH, ToMemoryStaticOperatorGraphRequest);
    jsonToReqFactory.emplace(REQ_RES_MEMORY_STATIC_OP_MEMORY_LIST, ToMemoryStaticOperatorListRequest);
    jsonToReqFactory.emplace(REQ_RES_MEMORY_STATIC_OP_MEMORY_MIN_MAX, ToMemoryStaticOperatorSizeRequest);
    jsonToReqFactory.emplace(REQ_RES_LEAKS_MEMORY_ALLOCATIONS, ToLeaksMemoryAllocationRequest);
    jsonToReqFactory.emplace(REQ_RES_LEAKS_MEMORY_BLOCKS, ToLeaksMemoryBlockRequest);
    jsonToReqFactory.emplace(REQ_RES_LEAKS_MEMORY_DETAILS, ToLeaksMemoryDetailRequest);
    jsonToReqFactory.emplace(REQ_RES_LEAKS_MEMORY_TRACES, ToLeaksMemoryTraceRequest);
}

void MemoryProtocol::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_RES_MEMORY_TYPE, ToMemoryTypeResponseJson);
    resToJsonFactory.emplace(REQ_RES_MEMORY_RESOURCE_TYPE, ToMemoryResourceTypeResponseJson);
    resToJsonFactory.emplace(REQ_RES_MEMORY_OPERATOR, ToMemoryOperatorResponseJson);
    resToJsonFactory.emplace(REQ_RES_MEMORY_COMPONENT, ToMemoryComponentResponseJson);
    resToJsonFactory.emplace(REQ_RES_MEMORY_VIEW, ToMemoryViewResponseJson);
    resToJsonFactory.emplace(REQ_RES_MEMORY_FIND_SLICE, ToMemoryFindSliceResponseJson);
    resToJsonFactory.emplace(REQ_RES_MEMORY_OPERATOR_MIN_MAX, ToMemoryOperatorSizeResponseJson);
    resToJsonFactory.emplace(REQ_RES_MEMORY_STATIC_OP_MEMORY_GRAPH, ToMemoryStaticOperatorGraphResponseJson);
    resToJsonFactory.emplace(REQ_RES_MEMORY_STATIC_OP_MEMORY_LIST, ToMemoryStaticOperatorListResponseJson);
    resToJsonFactory.emplace(REQ_RES_MEMORY_STATIC_OP_MEMORY_MIN_MAX, ToMemoryStaticOperatorSizeResponseJson);
    resToJsonFactory.emplace(REQ_RES_LEAKS_MEMORY_ALLOCATIONS, ToLeaksMemoryAllocationsResponse);
    resToJsonFactory.emplace(REQ_RES_LEAKS_MEMORY_BLOCKS, ToLeaksMemoryBlocksResponse);
    resToJsonFactory.emplace(REQ_RES_LEAKS_MEMORY_DETAILS, ToLeaksMemoryDetailsResponse);
    resToJsonFactory.emplace(REQ_RES_LEAKS_MEMORY_TRACES, ToLeaksMemoryTracesResponse);
}

void MemoryProtocol::RegisterEventToJsonFuncs()
{
    eventToJsonFactory.emplace(EVENT_MODULE_RESET, TimelineProtocol::ToModuleResetEventJson);
    eventToJsonFactory.emplace(EVENT_ALL_SUCCESS, TimelineProtocol::ToAllSuccessEventJson);
    eventToJsonFactory.emplace(EVENT_PARSE_LEAKS_MEMORY_COMPLETED, TimelineProtocol::ToLeaksParseSuccessEventJson);
}

#pragma region << Json To Request>>

std::unique_ptr<Request> MemoryProtocol::ToMemoryTypeRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<MemoryTypeRequest> reqPtr = std::make_unique<MemoryTypeRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->rankId, json["params"], "rankId");
    return reqPtr;
}

std::unique_ptr<Request> MemoryProtocol::ToMemoryResourceTypeRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<MemoryResourceTypeRequest> reqPtr = std::make_unique<MemoryResourceTypeRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->rankId, json["params"], "rankId");
    return reqPtr;
}

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
        reqPtr->params.startTime = JsonUtil::GetDouble(json["params"], "startTime");
    } else {
        reqPtr->params.startTime = -1;
    }
    if (json["params"].HasMember("endTime")) {
        reqPtr->params.endTime = JsonUtil::GetDouble(json["params"], "endTime");
    } else {
        reqPtr->params.endTime = -1;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isOnlyShowAllocatedOrReleasedWithinInterval,
        json["params"], "isOnlyShowAllocatedOrReleasedWithinInterval");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.currentPage, json["params"], "currentPage");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
    if (json["params"].HasMember("minSize")) {
        JsonUtil::SetByJsonKeyValue(reqPtr->params.minSize, json["params"], "minSize");
    } else {
        reqPtr->params.minSize = std::numeric_limits<int64_t>::min();
    }
    if (json["params"].HasMember("maxSize")) {
        JsonUtil::SetByJsonKeyValue(reqPtr->params.maxSize, json["params"], "maxSize");
    } else {
        reqPtr->params.maxSize = std::numeric_limits<int64_t>::max();
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.searchName, json["params"], "searchName");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isCompare, json["params"], "isCompare");
    return reqPtr;
}

std::unique_ptr<Request> MemoryProtocol::ToMemoryComponentRequest(const Dic::json_t &json, std::string &error)
{
    std::unique_ptr<MemoryComponentRequest> reqPtr = std::make_unique<MemoryComponentRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    if (!json.HasMember("params") || !json["params"].HasMember("rankId")) {
        error = "Request json lacks member rankId.";
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.currentPage, json["params"], "currentPage");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isCompare, json["params"], "isCompare");
    return reqPtr;
}

std::unique_ptr<Request> MemoryProtocol::ToLeaksMemoryAllocationRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<LeaksMemoryAllocationRequest> reqPtr = std::make_unique<LeaksMemoryAllocationRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    if (!json.HasMember("params") || !json["params"].HasMember("deviceId") ||
        !json["params"].HasMember("eventType")) {
        error = "Request[requestId=" + std::to_string(reqPtr->id) +
                "] json lacks member params or deviceId or eventType.";
        return nullptr;
    }
    const json_t &param_json = json["params"];
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTimestamp, param_json, "startTimestamp");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTimestamp, param_json, "endTimestamp");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.deviceId, param_json, "deviceId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.optimized, param_json, "optimized");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.relativeTime, param_json, "relativeTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.eventType, param_json, "eventType");
    return reqPtr;
}

std::unique_ptr<Request> MemoryProtocol::ToLeaksMemoryBlockRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<LeaksMemoryBlockRequest> reqPtr = std::make_unique<LeaksMemoryBlockRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    if (!json.HasMember("params") || !json["params"].HasMember("deviceId") ||
        !json["params"].HasMember("eventType")) {
        error = "Request[requestId=" + std::to_string(reqPtr->id) +
                "] json lacks member params or deviceId or eventType.";
        return nullptr;
    }
    const json_t &param_json = json["params"];
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTimestamp, param_json, "startTimestamp");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTimestamp, param_json, "endTimestamp");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTimestamp, param_json, "minSize");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTimestamp, param_json, "maxSize");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.deviceId, param_json, "deviceId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.relativeTime, param_json, "relativeTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.eventType, param_json, "eventType");
    return reqPtr;
}

std::unique_ptr<Request> MemoryProtocol::ToLeaksMemoryDetailRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<LeaksMemoryDetailRequest> reqPtr = std::make_unique<LeaksMemoryDetailRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    if (!json.HasMember("params") || !json["params"].HasMember("deviceId") ||
        !json["params"].HasMember("timestamp")) {
        error = "Request[requestId=" + std::to_string(reqPtr->id) +
                "] json lacks member params or deviceId or timestamp.";
        return nullptr;
    }
    const json_t &param_json = json["params"];
    JsonUtil::SetByJsonKeyValue(reqPtr->params.timestamp, param_json, "timestamp");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.deviceId, param_json, "deviceId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.relativeTime, param_json, "relativeTime");
    return reqPtr;
}

std::unique_ptr<Request> MemoryProtocol::ToLeaksMemoryTraceRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<LeaksMemoryTraceRequest> reqPtr = std::make_unique<LeaksMemoryTraceRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    if (!json.HasMember("params") || !json["params"].HasMember("deviceId") ||
        !json["params"].HasMember("threadId")) {
        error = "Request[requestId=" + std::to_string(reqPtr->id) +
                "] json lacks member params or deviceId or threadId.";
        return nullptr;
    }
    const json_t &param_json = json["params"];
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTimestamp, param_json, "startTimestamp");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTimestamp, param_json, "endTimestamp");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.relativeTime, param_json, "relativeTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.threadId, param_json, "threadId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.deviceId, param_json, "deviceId");
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
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isCompare, json["params"], "isCompare");
    return reqPtr;
}

std::unique_ptr<Request> MemoryProtocol::ToMemoryFindSliceRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<MemoryFindSliceRequest> reqPtr = std::make_unique<MemoryFindSliceRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.fileId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.id, json["params"], "id");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.name, json["params"], "name");
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
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isCompare, json["params"], "isCompare");
    return reqPtr;
}

std::unique_ptr<Request> MemoryProtocol::ToMemoryStaticOperatorGraphRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<MemoryStaticOperatorGraphRequest> reqPtr = std::make_unique<MemoryStaticOperatorGraphRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.modelName, json["params"], "modelName");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.graphId, json["params"], "graphId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isCompare, json["params"], "isCompare");
    return reqPtr;
}

std::unique_ptr<Request> MemoryProtocol::ToMemoryStaticOperatorListRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<MemoryStaticOperatorListRequest> reqPtr = std::make_unique<MemoryStaticOperatorListRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.graphId, json["params"], "graphId");

    if (json["params"].HasMember("startNodeIndex")) {
        reqPtr->params.startNodeIndex = JsonUtil::GetInteger(json["params"], "startNodeIndex");
    } else {
        reqPtr->params.startNodeIndex = -1;
    }
    if (json["params"].HasMember("endNodeIndex")) {
        reqPtr->params.endNodeIndex = JsonUtil::GetInteger(json["params"], "endNodeIndex");
    } else {
        reqPtr->params.endNodeIndex = -1;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.currentPage, json["params"], "currentPage");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
    if (json["params"].HasMember("minSize")) {
        JsonUtil::SetByJsonKeyValue(reqPtr->params.minSize, json["params"], "minSize");
    } else {
        reqPtr->params.minSize = std::numeric_limits<int64_t>::min();
    }
    if (json["params"].HasMember("maxSize")) {
        JsonUtil::SetByJsonKeyValue(reqPtr->params.maxSize, json["params"], "maxSize");
    } else {
        reqPtr->params.maxSize = std::numeric_limits<int64_t>::max();
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.searchName, json["params"], "searchName");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isCompare, json["params"], "isCompare");
    return reqPtr;
}

std::unique_ptr<Request> MemoryProtocol::ToMemoryStaticOperatorSizeRequest(const Dic::json_t &json, std::string &error)
{
    std::unique_ptr<MemoryStaticOperatorSizeRequest> reqPtr = std::make_unique<MemoryStaticOperatorSizeRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.graphId, json["params"], "graphId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isCompare, json["params"], "isCompare");
    return reqPtr;
}

#pragma endregion

#pragma region << Response To Json>>

std::optional<document_t> MemoryProtocol::ToMemoryTypeResponseJson(const Response &response)
{
    return ToResponseJson<MemoryTypeResponse>(dynamic_cast<const MemoryTypeResponse &>(response));
}

std::optional<document_t> MemoryProtocol::ToMemoryResourceTypeResponseJson(const Response &response)
{
    return ToResponseJson<MemoryResourceTypeResponse>(dynamic_cast<const MemoryResourceTypeResponse &>(response));
}

std::optional<document_t> MemoryProtocol::ToMemoryOperatorResponseJson(const Response &response)
{
    return ToResponseJson<MemoryOperatorComparisonResponse>(
        dynamic_cast<const MemoryOperatorComparisonResponse &>(response));
}

std::optional<document_t> MemoryProtocol::ToMemoryComponentResponseJson(const Response &response)
{
    return ToResponseJson<MemoryComponentComparisonResponse>(
        dynamic_cast<const MemoryComponentComparisonResponse &>(response));
}

std::optional<document_t> MemoryProtocol::ToMemoryViewResponseJson(const Response &response)
{
    return ToResponseJson<MemoryViewResponse>(dynamic_cast<const MemoryViewResponse &>(response));
}

std::optional<document_t> MemoryProtocol::ToMemoryFindSliceResponseJson(const Response &response)
{
    return ToResponseJson<MemoryFindSliceResponse>(dynamic_cast<const MemoryFindSliceResponse &>(response));
}

std::optional<document_t> MemoryProtocol::ToMemoryOperatorSizeResponseJson(const Response &response)
{
    return ToResponseJson<MemoryOperatorSizeResponse>(dynamic_cast<const MemoryOperatorSizeResponse &>(response));
}

std::optional<document_t> MemoryProtocol::ToMemoryStaticOperatorGraphResponseJson(const Response &response)
{
    return ToResponseJson<MemoryStaticOperatorGraphResponse>(
        dynamic_cast<const MemoryStaticOperatorGraphResponse &>(response));
}

std::optional<document_t> MemoryProtocol::ToMemoryStaticOperatorListResponseJson(const Response &response)
{
    return ToResponseJson<MemoryStaticOperatorListCompResponse>(
        dynamic_cast<const MemoryStaticOperatorListCompResponse &>(response));
}

std::optional<document_t> MemoryProtocol::ToMemoryStaticOperatorSizeResponseJson(const Response &response)
{
    return ToResponseJson<MemoryStaticOperatorSizeResponse>(
        dynamic_cast<const MemoryStaticOperatorSizeResponse &>(response));
}
std::optional<document_t> MemoryProtocol::ToLeaksMemoryAllocationsResponse(const Response &response)
{
    return ToResponseJson<LeaksMemoryAllocationsResponse>(
        dynamic_cast<const LeaksMemoryAllocationsResponse &>(response));
}

std::optional<document_t> MemoryProtocol::ToLeaksMemoryBlocksResponse(const Response &response)
{
    return ToResponseJson<LeaksMemoryBlocksResponse>(
            dynamic_cast<const LeaksMemoryBlocksResponse &>(response));
}

std::optional<document_t> MemoryProtocol::ToLeaksMemoryDetailsResponse(const Response &response)
{
    return ToResponseJson<LeaksMemoryDetailsResponse>(
            dynamic_cast<const LeaksMemoryDetailsResponse &>(response));
}

std::optional<document_t> MemoryProtocol::ToLeaksMemoryTracesResponse(const Response &response)
{
    return ToResponseJson<LeaksMemoryTracesResponse>(
            dynamic_cast<const LeaksMemoryTracesResponse &>(response));
}
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic
