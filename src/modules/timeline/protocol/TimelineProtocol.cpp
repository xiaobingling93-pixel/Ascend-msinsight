/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "TimelineProtocolUtil.h"
#include "TimelineProtocolRequest.h"
#include "TimelineProtocolResponse.h"
#include "TimelineProtocol.h"

namespace Dic {
namespace Protocol {
void TimelineProtocol::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_RES_IMPORT_ACTION, ToImportActionRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_THREAD_TRACES, ToUnitThreadTracesRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_THREADS, ToUnitThreadsRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_THREAD_DETAIL, ToThreadDetailRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_FLOW_NAME, ToUnitFlowNameRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_FLOW, ToUnitFlowRequest);
    jsonToReqFactory.emplace(REQ_RES_RESET_WINDOW, ToResetWindowRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_CHART, ToUnitChartRequest);
    jsonToReqFactory.emplace(REQ_RES_SEARCH_COUNT, ToSearchCountRequest);
    jsonToReqFactory.emplace(REQ_RES_SEARCH_SLICE, ToSearchSliceRequest);
    jsonToReqFactory.emplace(REQ_RES_REMOTE_DELETE, ToRemoteDeleteRequest);
    jsonToReqFactory.emplace(REQ_RES_FLOW_CATEGORY_LIST, ToFlowCategoryListRequest);
    jsonToReqFactory.emplace(REQ_RES_FLOW_CATEGORY_EVENTS, ToFlowCategoryEventsRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_COUNTER, ToUnitCounterRequest);
}

void TimelineProtocol::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_RES_IMPORT_ACTION, ToImportActionResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_THREAD_TRACES, ToUnitThreadTracesResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_THREADS, ToUnitThreadsResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_THREAD_DETAIL, ToThreadDetailResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_FLOW_NAME, ToUnitFlowNameResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_FLOW, ToUnitFlowResponseJson);
    resToJsonFactory.emplace(REQ_RES_RESET_WINDOW, ToResetWindowResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_CHART, ToUnitChartResponseJson);
    resToJsonFactory.emplace(REQ_RES_SEARCH_COUNT, ToSearchCountResponseJson);
    resToJsonFactory.emplace(REQ_RES_SEARCH_SLICE, ToSearchSliceResponseJson);
    resToJsonFactory.emplace(REQ_RES_REMOTE_DELETE, ToRemoteDeleteResponseJson);
    resToJsonFactory.emplace(REQ_RES_FLOW_CATEGORY_LIST, ToFlowCategoryListResponse);
    resToJsonFactory.emplace(REQ_RES_FLOW_CATEGORY_EVENTS, ToFlowCategoryEventsResponse);
    resToJsonFactory.emplace(REQ_RES_UNIT_COUNTER, ToUnitCounterResponse);
}

void TimelineProtocol::RegisterEventToJsonFuncs()
{
    eventToJsonFactory.emplace(EVENT_PARSE_SUCCESS, ToParseSuccessEventJson);
    eventToJsonFactory.emplace(EVENT_PARSE_FAIL, ToParseFailEventJson);
}

#pragma region <<Json To Request>>

std::unique_ptr<Request> TimelineProtocol::ToImportActionRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<ImportActionRequest> reqPtr = std::make_unique<ImportActionRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    if (json["params"].contains("path") && json["params"]["path"].is_array()) {
        for (const auto &path : json["params"]["path"]) {
            reqPtr->params.path.emplace_back(path);
        }
    }
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToUnitThreadTracesRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<UnitThreadTracesRequest> reqPtr = std::make_unique<UnitThreadTracesRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.cardId, json["params"], "cardId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.processId, json["params"], "processId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.threadId, json["params"], "threadId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTime, json["params"], "startTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTime, json["params"], "endTime");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToUnitThreadsRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<UnitThreadsRequest> reqPtr = std::make_unique<UnitThreadsRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.tid, json["params"], "tid");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pid, json["params"], "pid");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTime, json["params"], "startTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTime, json["params"], "endTime");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToThreadDetailRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<ThreadDetailRequest> reqPtr = std::make_unique<ThreadDetailRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.tid, json["params"], "tid");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pid, json["params"], "pid");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTime, json["params"], "startTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.depth, json["params"], "depth");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToUnitFlowNameRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<UnitFlowNameRequest> reqPtr = std::make_unique<UnitFlowNameRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.tid, json["params"], "tid");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pid, json["params"], "pid");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTime, json["params"], "startTime");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToUnitFlowRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<UnitFlowRequest> reqPtr = std::make_unique<UnitFlowRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.flowId, json["params"], "flowId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTime, json["params"], "startTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.type, json["params"], "type");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToResetWindowRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<ResetWindowRequest> reqPtr = std::make_unique<ResetWindowRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToUnitChartRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<UnitChartRequest> reqPtr = std::make_unique<UnitChartRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.param, json["params"], "param");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToSearchCountRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<SearchCountRequest> reqPtr = std::make_unique<SearchCountRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.searchContent, json["params"], "searchContent");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToSearchSliceRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<SearchSliceRequest> reqPtr = std::make_unique<SearchSliceRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.searchContent, json["params"], "searchContent");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.index, json["params"], "index");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToRemoteDeleteRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<RemoteDeleteRequest> reqPtr = std::make_unique<RemoteDeleteRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    if (json["params"].contains("rankId") && json["params"]["rankId"].is_array()) {
        for (const auto &id : json["params"]["rankId"]) {
            reqPtr->params.rankId.emplace_back(id);
        }
    }
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToFlowCategoryListRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<FlowCategoryListRequest> reqPtr = std::make_unique<FlowCategoryListRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToFlowCategoryEventsRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<FlowCategoryEventsRequest> reqPtr = std::make_unique<FlowCategoryEventsRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.category, json["params"], "category");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTime, json["params"], "startTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTime, json["params"], "endTime");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToUnitCounterRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<UnitCounterRequest> reqPtr = std::make_unique<UnitCounterRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pid, json["params"], "pid");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.threadName, json["params"], "threadName");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTime, json["params"], "startTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTime, json["params"], "endTime");
    return reqPtr;
}
#pragma endregion

#pragma region <<Response To Json>>

std::optional<json_t> TimelineProtocol::ToImportActionResponseJson(const Response &response)
{
    return ToResponseJson<ImportActionResponse>(dynamic_cast<const ImportActionResponse &>(response));
}

std::optional<json_t> TimelineProtocol::ToUnitThreadTracesResponseJson(const Response &response)
{
    return ToResponseJson<UnitThreadTracesResponse>(dynamic_cast<const UnitThreadTracesResponse &>(response));
}

std::optional<json_t> TimelineProtocol::ToUnitThreadsResponseJson(const Response &response)
{
    return ToResponseJson<UnitThreadsResponse>(dynamic_cast<const UnitThreadsResponse &>(response));
}

std::optional<json_t> TimelineProtocol::ToThreadDetailResponseJson(const Response &response)
{
    return ToResponseJson<UnitThreadDetailResponse>(dynamic_cast<const UnitThreadDetailResponse &>(response));
}

std::optional<json_t> TimelineProtocol::ToUnitFlowNameResponseJson(const Response &response)
{
    return ToResponseJson<UnitFlowNameResponse>(dynamic_cast<const UnitFlowNameResponse &>(response));
}

std::optional<json_t> TimelineProtocol::ToUnitFlowResponseJson(const Response &response)
{
    return ToResponseJson<UnitFlowResponse>(dynamic_cast<const UnitFlowResponse &>(response));
}

std::optional<json_t> TimelineProtocol::ToResetWindowResponseJson(const Response &response)
{
    return ToResponseJson<ResetWindowResponse>(dynamic_cast<const ResetWindowResponse &>(response));
}

std::optional<json_t> TimelineProtocol::ToUnitChartResponseJson(const Response &response)
{
    return ToResponseJson<UnitChartResponse>(dynamic_cast<const UnitChartResponse &>(response));
}

std::optional<json_t> TimelineProtocol::ToSearchCountResponseJson(const Response &response)
{
    return ToResponseJson<SearchCountResponse>(dynamic_cast<const SearchCountResponse &>(response));
}

std::optional<json_t> TimelineProtocol::ToSearchSliceResponseJson(const Response &response)
{
    return ToResponseJson<SearchSliceResponse>(dynamic_cast<const SearchSliceResponse &>(response));
}

std::optional<json_t> TimelineProtocol::ToRemoteDeleteResponseJson(const Response &response)
{
    return ToResponseJson<RemoteDeleteResponse>(dynamic_cast<const RemoteDeleteResponse &>(response));
}

std::optional<json_t> TimelineProtocol::ToFlowCategoryListResponse(const Response &response)
{
    return ToResponseJson<FlowCategoryListResponse>(dynamic_cast<const FlowCategoryListResponse &>(response));
}

std::optional<json_t> TimelineProtocol::ToFlowCategoryEventsResponse(const Response &response)
{
    return ToResponseJson<FlowCategoryEventsResponse>(dynamic_cast<const FlowCategoryEventsResponse &>(response));
}

std::optional<json_t> TimelineProtocol::ToUnitCounterResponse(const Response &response)
{
    return ToResponseJson<UnitCounterResponse>(dynamic_cast<const UnitCounterResponse &>(response));
}
#pragma endregion

#pragma region <<Event To Json>>
std::optional<json_t> TimelineProtocol::ToParseSuccessEventJson(const Event &event)
{
    return ToEventJson<ParseSuccessEvent>(dynamic_cast<const ParseSuccessEvent &>(event));
}
std::optional<json_t> TimelineProtocol::ToParseFailEventJson(const Event &event)
{
    return ToEventJson<ParseFailEvent>(dynamic_cast<const ParseFailEvent &>(event));
}
#pragma endregion
} // namespace Protocol
} // namespace Dic
