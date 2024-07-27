/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "pch.h"
#include "TimelineProtocolUtil.h"
#include "TimelineProtocolRequest.h"
#include "TimelineProtocolResponse.h"
#include "TimelineProtocol.h"

namespace Dic {
namespace Protocol {
void TimelineProtocol::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_RES_IMPORT_ACTION, ToImportActionRequest);
    jsonToReqFactory.emplace(REQ_RES_PARSE_CARDS, ToParseCardsRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_THREAD_TRACES, ToUnitThreadTracesRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_THREAD_TRACES_SUMMARY, ToUnitThreadTracesSummaryRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_THREADS, ToUnitThreadsRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_THREAD_DETAIL, ToThreadDetailRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_FLOW_NAME, ToUnitFlowNameRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_FLOWS, ToUnitFlowsRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_FLOW, ToUnitFlowRequest);
    jsonToReqFactory.emplace(REQ_RES_RESET_WINDOW, ToResetWindowRequest);
    jsonToReqFactory.emplace(REQ_RES_SEARCH_COUNT, ToSearchCountRequest);
    jsonToReqFactory.emplace(REQ_RES_SEARCH_SLICE, ToSearchSliceRequest);
    jsonToReqFactory.emplace(REQ_RES_REMOTE_DELETE, ToRemoteDeleteRequest);
    jsonToReqFactory.emplace(REQ_RES_FLOW_CATEGORY_LIST, ToFlowCategoryListRequest);
    jsonToReqFactory.emplace(REQ_RES_FLOW_CATEGORY_EVENTS, ToFlowCategoryEventsRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_COUNTER, ToUnitCounterRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_SYSTEM_VIEW, ToSystemViewRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_EVENTS_VIEW, ToEventsViewRequest);
    jsonToReqFactory.emplace(REQ_RES_UNIT_KERNEL_DETAILS, ToKernelDetailRequest);
    jsonToReqFactory.emplace(REQ_RES_ONE_KERNEL_DETAILS, ToOneKernelRequest);
    jsonToReqFactory.emplace(REQ_RES_SAME_OPERATORS_DURATION, ToUnitThreadsOperatorsRequest);
    jsonToReqFactory.emplace(REQ_RES_UPLOAD_FILE, ToUploadFileRequest);
    jsonToReqFactory.emplace(REQ_RES_SEARCH_ALL_SLICES, ToSearchAllSlicesRequest);
}

void TimelineProtocol::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_RES_IMPORT_ACTION, ToImportActionResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_THREAD_TRACES, ToUnitThreadTracesResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_THREAD_TRACES_SUMMARY, ToUnitThreadTracesSummaryResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_THREADS, ToUnitThreadsResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_THREAD_DETAIL, ToThreadDetailResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_FLOW_NAME, ToUnitFlowNameResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_FLOW, ToUnitFlowResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_FLOWS, ToUnitFlowsResponseJson);
    resToJsonFactory.emplace(REQ_RES_RESET_WINDOW, ToResetWindowResponseJson);
    resToJsonFactory.emplace(REQ_RES_SEARCH_COUNT, ToSearchCountResponseJson);
    resToJsonFactory.emplace(REQ_RES_SEARCH_SLICE, ToSearchSliceResponseJson);
    resToJsonFactory.emplace(REQ_RES_REMOTE_DELETE, ToRemoteDeleteResponseJson);
    resToJsonFactory.emplace(REQ_RES_FLOW_CATEGORY_LIST, ToFlowCategoryListResponse);
    resToJsonFactory.emplace(REQ_RES_FLOW_CATEGORY_EVENTS, ToFlowCategoryEventsResponse);
    resToJsonFactory.emplace(REQ_RES_UNIT_COUNTER, ToUnitCounterResponse);
    resToJsonFactory.emplace(REQ_RES_UNIT_SYSTEM_VIEW, ToSystemViewResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_EVENTS_VIEW, ToEventsViewResponseJson);
    resToJsonFactory.emplace(REQ_RES_UNIT_KERNEL_DETAILS, ToKernelDetailResponseJson);
    resToJsonFactory.emplace(REQ_RES_ONE_KERNEL_DETAILS, ToOneKernelResponseJson);
    resToJsonFactory.emplace(REQ_RES_SAME_OPERATORS_DURATION, ToUnitThreadsOperatorsResponseJson);
    resToJsonFactory.emplace(REQ_RES_UPLOAD_FILE, ToUploadFileResponseJson);
    resToJsonFactory.emplace(REQ_RES_SEARCH_ALL_SLICES, ToSearchAllSlicesResponseJson);
    resToJsonFactory.emplace(REQ_RES_PARSE_CARDS, ToParseCardsResponseJson);
}

void TimelineProtocol::RegisterEventToJsonFuncs()
{
    eventToJsonFactory.emplace(EVENT_PARSE_SUCCESS, ToParseSuccessEventJson);
    eventToJsonFactory.emplace(EVENT_PARSE_FAIL, ToParseFailEventJson);
    eventToJsonFactory.emplace(EVENT_PARSE_CLUSTER_COMPLETED, ToParseClusterCompletedEventJson);
    eventToJsonFactory.emplace(EVENT_PARSE_CLUSTER_STEP2_COMPLETED, ToParseClusterStep2CompletedEventJson);
    eventToJsonFactory.emplace(EVENT_PARSE_MEMORY_COMPLETED, ToParseMemoryCompletedEventJson);
    eventToJsonFactory.emplace(EVENT_MODULE_RESET, ToModuleResetEventJson);
    eventToJsonFactory.emplace(EVENT_PARSE_PROGRESS, ToParseProgressEventJson);
}

#pragma region << Json To Request>>

std::unique_ptr<Request> TimelineProtocol::ToImportActionRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<ImportActionRequest> reqPtr = std::make_unique<ImportActionRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    if (json["params"].HasMember("path") && json["params"]["path"].IsArray()) {
        for (const auto &path : json["params"]["path"].GetArray()) {
            reqPtr->params.path.emplace_back(path.GetString());
        }
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.projectName, json["params"], "projectName");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToParseCardsRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<ParseCardsRequest> reqPtr = std::make_unique<ParseCardsRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    if (json["params"].HasMember("cards") && json["params"]["cards"].IsArray()) {
        for (const auto &card : json["params"]["cards"].GetArray()) {
            reqPtr->params.cards.emplace_back(card.GetString());
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
    JsonUtil::SetByJsonKeyValue(reqPtr->params.metaType, json["params"], "metaType");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.timePerPx, json["params"], "timePerPx");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isFilterPythonFunction, json["params"], "isFilterPythonFunction");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isHideFlagEvents, json["params"], "isHideFlagEvents");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTime, json["params"], "startTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTime, json["params"], "endTime");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToUnitThreadTracesSummaryRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<UnitThreadTracesSummaryRequest> reqPtr = std::make_unique<UnitThreadTracesSummaryRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.cardId, json["params"], "cardId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.processId, json["params"], "processId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTime, json["params"], "startTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.metaType, json["params"], "metaType");
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
    JsonUtil::SetByJsonKeyValue(reqPtr->params.metaType, json["params"], "metaType");
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
    JsonUtil::SetByJsonKeyValue(reqPtr->params.metaType, json["params"], "metaType");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.id, json["params"], "id");
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
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTime, json["params"], "endTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.metaType, json["params"], "metaType");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.id, json["params"], "id");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToUnitFlowsRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<UnitFlowsRequest> reqPtr = std::make_unique<UnitFlowsRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.tid, json["params"], "tid");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pid, json["params"], "pid");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTime, json["params"], "startTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTime, json["params"], "endTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.id, json["params"], "id");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.metaType, json["params"], "metaType");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isSimulation, json["params"], "isSimulation");
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
    JsonUtil::SetByJsonKeyValue(reqPtr->params.id, json["params"], "id");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.metaType, json["params"], "metaType");
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

std::unique_ptr<Request> TimelineProtocol::ToSearchCountRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<SearchCountRequest> reqPtr = std::make_unique<SearchCountRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isMatchCase, json["params"], "isMatchCase");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isMatchExact, json["params"], "isMatchExact");
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
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isMatchCase, json["params"], "isMatchCase");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isMatchExact, json["params"], "isMatchExact");
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
    if (json["params"].HasMember("rankId") && json["params"]["rankId"].IsArray()) {
        for (const auto &id : json["params"]["rankId"].GetArray()) {
            reqPtr->params.rankId.emplace_back(id.GetString());
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
    JsonUtil::SetByJsonKeyValue(reqPtr->params.host, json["params"], "host");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.category, json["params"], "category");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.timePerPx, json["params"], "timePerPx");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isSimulation, json["params"], "isSimulation");
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
    JsonUtil::SetByJsonKeyValue(reqPtr->params.threadId, json["params"], "threadId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.metaType, json["params"], "metaType");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToSystemViewRequest(const Dic::json_t &json, std::string &error)
{
    std::unique_ptr<SystemViewRequest> reqPtr = std::make_unique<SystemViewRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.type, json["params"], "type");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTime, json["params"], "endTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.current, json["params"], "current");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isQueryTotal, json["params"], "isQueryTotal");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.layer, json["params"], "layer");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.searchName, json["params"], "searchName");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToEventsViewRequest(const Dic::json_t &json, std::string &error)
{
    auto reqPtr = std::make_unique<EventsViewRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.currentPage, json["params"], "currentPage");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pid, json["params"], "pid");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.processName, json["params"], "processName");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.tid, json["params"], "tid");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.threadName, json["params"], "threadName");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.metaType, json["params"], "metaType");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToKernelDetailRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<KernelDetailsRequest> reqPtr = std::make_unique<KernelDetailsRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.current, json["params"], "current");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.coreType, json["params"], "coreType");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.searchName, json["params"], "searchName");
    if (json["params"].HasMember("filterCondition") && json["params"]["filterCondition"].IsArray()) {
        for (const auto &filter : json["params"]["filterCondition"].GetArray()) {
            if (filter.IsString()) {
                std::pair<std::string, std::string> pFilter("", "");
                auto fil = JsonUtil::TryParse(filter.GetString(), error);
                pFilter.first = JsonUtil::GetString(fil->GetObj(), "columnName");
                pFilter.second = JsonUtil::GetString(fil->GetObj(), "value");
                reqPtr->params.filters.emplace_back(pFilter);
            }
        }
    }
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToOneKernelRequest(const Dic::json_t &json, std::string &error)
{
    std::unique_ptr<KernelRequest> reqPtr = std::make_unique<KernelRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.name, json["params"], "name");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.timestamp, json["params"], "timestamp");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.duration, json["params"], "duration");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToUploadFileRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<UploadFileRequest> reqPtr = std::make_unique<UploadFileRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.text, json["params"], "text");

    JsonUtil::SetByJsonKeyValue(reqPtr->params.fileAttr.count, json["params"]["fileAttr"], "count");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.fileAttr.index, json["params"]["fileAttr"], "index");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.fileAttr.isInFolder, json["params"]["fileAttr"], "isInFolder");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.fileAttr.isLast, json["params"]["fileAttr"], "isLast");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.fileAttr.name, json["params"]["fileAttr"], "name");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.fileAttr.path, json["params"]["fileAttr"], "path");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.fileAttr.size, json["params"]["fileAttr"], "size");

    JsonUtil::SetByJsonKeyValue(reqPtr->params.slice.count, json["params"]["slice"], "count");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.slice.index, json["params"]["slice"], "index");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.slice.isLast, json["params"]["slice"], "isLast");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.slice.isSliced, json["params"]["slice"], "isSliced");

    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToUnitThreadsOperatorsRequest(const Dic::json_t &json, std::string &error)
{
    std::unique_ptr<UnitThreadsOperatorsRequest> reqPtr = std::make_unique<UnitThreadsOperatorsRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.tid, json["params"], "tid");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pid, json["params"], "pid");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.startTime, json["params"], "startTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.endTime, json["params"], "endTime");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.name, json["params"], "name");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.current, json["params"], "current");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.metaType, json["params"], "metaType");
    return reqPtr;
}

std::unique_ptr<Request> TimelineProtocol::ToSearchAllSlicesRequest(const Dic::json_t &json, std::string &error)
{
    std::unique_ptr<SearchAllSlicesRequest> reqPtr = std::make_unique<SearchAllSlicesRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isMatchCase, json["params"], "isMatchCase");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isMatchExact, json["params"], "isMatchExact");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.searchContent, json["params"], "searchContent");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.current, json["params"], "current");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
    return reqPtr;
}
#pragma endregion

#pragma region << Response To Json>>

std::optional<document_t> TimelineProtocol::ToImportActionResponseJson(const Response &response)
{
    return ToResponseJson<ImportActionResponse>(dynamic_cast<const ImportActionResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToUnitThreadTracesResponseJson(const Response &response)
{
    return ToResponseJson<UnitThreadTracesResponse>(dynamic_cast<const UnitThreadTracesResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToUnitThreadTracesSummaryResponseJson(const Response &response)
{
    return ToResponseJson<UnitThreadTracesSummaryResponse>(
        dynamic_cast<const UnitThreadTracesSummaryResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToUnitThreadsResponseJson(const Response &response)
{
    return ToResponseJson<UnitThreadsResponse>(dynamic_cast<const UnitThreadsResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToThreadDetailResponseJson(const Response &response)
{
    return ToResponseJson<UnitThreadDetailResponse>(dynamic_cast<const UnitThreadDetailResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToUnitFlowNameResponseJson(const Response &response)
{
    return ToResponseJson<UnitFlowNameResponse>(dynamic_cast<const UnitFlowNameResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToUnitFlowResponseJson(const Response &response)
{
    return ToResponseJson<UnitFlowResponse>(dynamic_cast<const UnitFlowResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToUnitFlowsResponseJson(const Response &response)
{
    return ToResponseJson<UnitFlowsResponse>(dynamic_cast<const UnitFlowsResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToResetWindowResponseJson(const Response &response)
{
    return ToResponseJson<ResetWindowResponse>(dynamic_cast<const ResetWindowResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToSearchCountResponseJson(const Response &response)
{
    return ToResponseJson<SearchCountResponse>(dynamic_cast<const SearchCountResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToSearchSliceResponseJson(const Response &response)
{
    return ToResponseJson<SearchSliceResponse>(dynamic_cast<const SearchSliceResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToRemoteDeleteResponseJson(const Response &response)
{
    return ToResponseJson<RemoteDeleteResponse>(dynamic_cast<const RemoteDeleteResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToFlowCategoryListResponse(const Response &response)
{
    return ToResponseJson<FlowCategoryListResponse>(dynamic_cast<const FlowCategoryListResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToFlowCategoryEventsResponse(const Response &response)
{
    return ToResponseJson<FlowCategoryEventsResponse>(dynamic_cast<const FlowCategoryEventsResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToUnitCounterResponse(const Response &response)
{
    return ToResponseJson<UnitCounterResponse>(dynamic_cast<const UnitCounterResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToSystemViewResponseJson(const Dic::Protocol::Response &response)
{
    return ToResponseJson<SystemViewResponse>(dynamic_cast<const SystemViewResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToEventsViewResponseJson(const Dic::Protocol::Response &response)
{
    return ToResponseJson<EventsViewResponse>(dynamic_cast<const EventsViewResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToKernelDetailResponseJson(const Dic::Protocol::Response &response)
{
    return ToResponseJson<KernelDetailsResponse>(dynamic_cast<const KernelDetailsResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToOneKernelResponseJson(const Dic::Protocol::Response &response)
{
    return ToResponseJson<OneKernelResponse>(dynamic_cast<const OneKernelResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToUnitThreadsOperatorsResponseJson(const Dic::Protocol::Response &response)
{
    return ToResponseJson<UnitThreadsOperatorsResponse>(dynamic_cast<const UnitThreadsOperatorsResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToUploadFileResponseJson(const Response &response)
{
    return ToResponseJson<UploadFileResponse>(dynamic_cast<const UploadFileResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToSearchAllSlicesResponseJson(const Response &response)
{
    return ToResponseJson<SearchAllSlicesResponse>(dynamic_cast<const SearchAllSlicesResponse &>(response));
}

std::optional<document_t> TimelineProtocol::ToParseCardsResponseJson(const Response &response)
{
    return ToResponseJson<ParseCardsResponse>(dynamic_cast<const ParseCardsResponse &>(response));
}
#pragma endregion

#pragma region << Event To Json>>
std::optional<document_t> TimelineProtocol::ToParseSuccessEventJson(const Event &event)
{
    return ToEventJson<ParseSuccessEvent>(dynamic_cast<const ParseSuccessEvent &>(event));
}

std::optional<document_t> TimelineProtocol::ToParseFailEventJson(const Event &event)
{
    return ToEventJson<ParseFailEvent>(dynamic_cast<const ParseFailEvent &>(event));
}

std::optional<document_t> TimelineProtocol::ToParseClusterCompletedEventJson(const Event &event)
{
    return ToEventJson<ParseClusterCompletedEvent>(dynamic_cast<const ParseClusterCompletedEvent &>(event));
}

std::optional<document_t> TimelineProtocol::ToAllSuccessEventJson(const Event &event)
{
    return ToEventJson<AllSuccessEvent>(dynamic_cast<const AllSuccessEvent &>(event));
}

std::optional<document_t> TimelineProtocol::ToParseClusterStep2CompletedEventJson(const Event &event)
{
    return ToEventJson<ParseClusterStep2CompletedEvent>(dynamic_cast<const ParseClusterStep2CompletedEvent &>(event));
}

std::optional<document_t> TimelineProtocol::ToParseMemoryCompletedEventJson(const Event &event)
{
    return ToEventJson<ParseMemoryCompletedEvent>(dynamic_cast<const ParseMemoryCompletedEvent &>(event));
}

std::optional<document_t> TimelineProtocol::ToModuleResetEventJson(const Event &event)
{
    return ToEventJson<ModuleResetEvent>(dynamic_cast<const ModuleResetEvent &>(event));
}

std::optional<document_t> TimelineProtocol::ToParseProgressEventJson(const Event &event)
{
    return ToEventJson<ParseProgressEvent>(dynamic_cast<const ParseProgressEvent &>(event));
}

#pragma endregion
} // namespace Protocol
} // namespace Dic
