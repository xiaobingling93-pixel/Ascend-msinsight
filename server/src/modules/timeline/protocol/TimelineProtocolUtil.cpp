/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "ServerLog.h"
#include "JsonUtil.h"
#include "ProtocolEnumUtil.h"
#include "TimelineProtocol.h"
#include "TimelineProtocolUtil.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
using namespace rapidjson;
#pragma region <<Response to json>>
template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("ToResponseJson is not implemented. command:", response.command);
    return std::nullopt;
}

template <> std::optional<document_t> ToResponseJson<ImportActionResponse>(const ImportActionResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "isCluster", response.body.isCluster, allocator);
    JsonUtil::AddMember(body, "reset", response.body.reset, allocator);
    json_t result(kArrayType);
    for (const Action& action : response.body.result) {
        json_t actionJson(kObjectType);
        JsonUtil::AddMember(actionJson, "cardName", action.cardName, allocator);
        JsonUtil::AddMember(actionJson, "rankId", action.rankId, allocator);
        JsonUtil::AddMember(actionJson, "result", action.result, allocator);
        result.PushBack(actionJson, allocator);
    }
    JsonUtil::AddMember(body, "result", result, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<UnitThreadTracesResponse>(const UnitThreadTracesResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t data(kArrayType);
    for (const std::vector<ThreadTraces>& array : response.body.data) {
        json_t threadTracesArray(kArrayType);
        for (const ThreadTraces& threadTraces : array) {
            json_t threadJson(kObjectType);
            JsonUtil::AddMember(threadJson, "name", threadTraces.name, allocator);
            JsonUtil::AddMember(threadJson, "duration", threadTraces.duration, allocator);
            JsonUtil::AddMember(threadJson, "startTime", threadTraces.startTime, allocator);
            JsonUtil::AddMember(threadJson, "endTime", threadTraces.endTime, allocator);
            JsonUtil::AddMember(threadJson, "depth", threadTraces.depth, allocator);
            JsonUtil::AddMember(threadJson, "threadId", threadTraces.threadId, allocator);
            threadTracesArray.PushBack(threadJson, allocator);
        }
        data.PushBack(threadTracesArray, allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<UnitThreadsResponse>(const UnitThreadsResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "emptyFlag", response.body.emptyFlag, allocator);
    json_t data(kArrayType);
    for (const Threads& threads : response.body.data) {
        json_t threadsJson(kObjectType);
        JsonUtil::AddMember(threadsJson, "title", threads.title, allocator);
        JsonUtil::AddMember(threadsJson, "wallDuration", threads.wallDuration, allocator);
        JsonUtil::AddMember(threadsJson, "occurrences", threads.occurrences, allocator);
        JsonUtil::AddMember(threadsJson, "avgWallDuration", threads.avgWallDuration, allocator);
        JsonUtil::AddMember(threadsJson, "selfTime", threads.selfTime, allocator);
        data.PushBack(threadsJson, allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<UnitThreadDetailResponse>(const UnitThreadDetailResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "emptyFlag", response.body.emptyFlag, allocator);
    json_t data(kObjectType);
    JsonUtil::AddMember(data, "selfTime", response.body.data.selfTime, allocator);
    JsonUtil::AddMember(data, "args", response.body.data.args, allocator);
    JsonUtil::AddMember(data, "title", response.body.data.title, allocator);
    JsonUtil::AddMember(data, "duration", response.body.data.duration, allocator);
    JsonUtil::AddMember(data, "cat", response.body.data.cat, allocator);
    JsonUtil::AddMember(body, "data", data, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<UnitFlowNameResponse>(const UnitFlowNameResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t flowDetail(kArrayType);
    for (const FlowName& flowName : response.body.flowDetail) {
        json_t flowJson(kObjectType);
        JsonUtil::AddMember(flowJson, "title", flowName.title, allocator);
        JsonUtil::AddMember(flowJson, "flowId", flowName.flowId, allocator);
        JsonUtil::AddMember(flowJson, "type", flowName.type, allocator);
        flowDetail.PushBack(flowJson, allocator);
    }
    JsonUtil::AddMember(body, "flowDetail", flowDetail, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

json_t FlowLocationToJson(const FlowLocation& flowLocation, RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
{
    json_t json(kObjectType);
    JsonUtil::AddMember(json, "pid", flowLocation.pid, allocator);
    JsonUtil::AddMember(json, "tid", flowLocation.tid, allocator);
    JsonUtil::AddMember(json, "timestamp", flowLocation.timestamp, allocator);
    JsonUtil::AddMember(json, "duration", flowLocation.duration, allocator);
    JsonUtil::AddMember(json, "depth", flowLocation.depth, allocator);
    JsonUtil::AddMember(json, "name", flowLocation.name, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<UnitFlowResponse>(const UnitFlowResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "title", response.body.title, allocator);
    JsonUtil::AddMember(body, "cat", response.body.cat, allocator);
    JsonUtil::AddMember(body, "id", response.body.id, allocator);
    JsonUtil::AddMember(body, "from", FlowLocationToJson(response.body.from, allocator), allocator);
    JsonUtil::AddMember(body, "to", FlowLocationToJson(response.body.to, allocator), allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<ResetWindowResponse>(const ResetWindowResponse &response)
{
    document_t json(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<UnitChartResponse>(const UnitChartResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t data(kArrayType);
    for (const Chart& chart: response.body.data) {
        json_t chartJson(kObjectType);
        JsonUtil::AddMember(chartJson, "ts", chart.ts, allocator);
        JsonUtil::AddMember(chartJson, "value", chart.value, allocator);
        data.PushBack(chartJson, allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<SearchCountResponse>(const SearchCountResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "totalCount", response.body.totalCount, allocator);
    json_t countList(kArrayType);
    for (const auto &data : response.body.countList) {
        json_t tmp(kObjectType);
        JsonUtil::AddMember(tmp, "rankId", data.rankId, allocator);
        JsonUtil::AddMember(tmp, "count", data.count, allocator);
        countList.PushBack(tmp, allocator);
    }
    JsonUtil::AddMember(body, "countList", countList, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<SearchSliceResponse>(const SearchSliceResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "rankId", response.body.rankId, allocator);
    JsonUtil::AddMember(body, "pid", response.body.pid, allocator);
    JsonUtil::AddMember(body, "tid", response.body.tid, allocator);
    JsonUtil::AddMember(body, "startTime", response.body.startTime, allocator);
    JsonUtil::AddMember(body, "duration", response.body.duration, allocator);
    JsonUtil::AddMember(body, "depth", response.body.depth, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<RemoteDeleteResponse>(const RemoteDeleteResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "startTimeUpdated", response.body.startTimeUpdated, allocator);
    JsonUtil::AddMember(body, "maxTimeStamp", response.body.maxTimeStamp, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<FlowCategoryListResponse>(const FlowCategoryListResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t category(kArrayType);
    for (const std::string &cat : response.body.category) {
        category.PushBack(json_t().SetString(cat.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(body, "category", category, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

json_t FlowEventLocationToJson(const FlowEventLocation& flowLocation, RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
{
    json_t json(kObjectType);
    JsonUtil::AddMember(json, "pid", flowLocation.pid, allocator);
    JsonUtil::AddMember(json, "tid", flowLocation.tid, allocator);
    JsonUtil::AddMember(json, "timestamp", flowLocation.timestamp, allocator);
    JsonUtil::AddMember(json, "depth", flowLocation.depth, allocator);
    return std::move(json);
}

template <>
std::optional<document_t> ToResponseJson<FlowCategoryEventsResponse>(const FlowCategoryEventsResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t flowDetailList(kArrayType);
    for (const auto &flowDetail : response.body.flowDetailList) {
        json_t flowDetailJson(kObjectType);
        JsonUtil::AddMember(flowDetailJson, "category", flowDetail->category, allocator);
        JsonUtil::AddMember(flowDetailJson, "from", FlowEventLocationToJson(flowDetail->from, allocator), allocator);
        JsonUtil::AddMember(flowDetailJson, "to", FlowEventLocationToJson(flowDetail->to, allocator), allocator);
        flowDetailList.PushBack(flowDetailJson, allocator);
    }
    JsonUtil::AddMember(body, "flowDetailList", flowDetailList, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<UnitCounterResponse>(const UnitCounterResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t data(kArrayType);
    std::string error;
    for (const auto &counterData : response.body.data) {
        json_t tmp(kObjectType);
        JsonUtil::AddMember(tmp, "timestamp", counterData.timestamp, allocator);
        auto value = JsonUtil::TryParse(counterData.valueJsonStr, error);
        if (value.has_value()) {
            JsonUtil::AddMember(tmp, "value", value.value(), allocator);
        } else {
            ServerLog::Warn("Failed to parse unit counter value. ", error);
        }
        data.PushBack(tmp, allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<SystemViewResponse>(const SystemViewResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t details(kArrayType);
    for (const SystemViewDetail& systemView : response.body.systemViewDetail) {
        json_t item(kObjectType);
        JsonUtil::AddMember(item, "name", systemView.name, allocator);
        JsonUtil::AddMember(item, "time", systemView.time, allocator);
        JsonUtil::AddMember(item, "totalTime", systemView.totalTime, allocator);
        JsonUtil::AddMember(item, "numberCalls", systemView.numberCalls, allocator);
        JsonUtil::AddMember(item, "avg", systemView.avg, allocator);
        JsonUtil::AddMember(item, "min", systemView.min, allocator);
        JsonUtil::AddMember(item, "max", systemView.max, allocator);
        details.PushBack(item, allocator);
    }
    JsonUtil::AddMember(body, "systemViewDetails", details, allocator);
    JsonUtil::AddMember(body, "count", response.body.total, allocator);
    JsonUtil::AddMember(body, "pageSize", response.body.pageSize, allocator);
    JsonUtil::AddMember(body, "currentPage", response.body.currentPage, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<KernelDetailsResponse>(const KernelDetailsResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t details(kArrayType);
    for (const KernelDetail& item : response.body.kernelDetails) {
        json_t kernelJson(kObjectType);
        JsonUtil::AddMember(kernelJson, "name", item.name, allocator);
        JsonUtil::AddMember(kernelJson, "type", item.type, allocator);
        JsonUtil::AddMember(kernelJson, "acceleratorCore", item.acceleratorCore, allocator);
        JsonUtil::AddMember(kernelJson, "startTime", item.startTime, allocator);
        JsonUtil::AddMember(kernelJson, "duration", item.duration, allocator);
        JsonUtil::AddMember(kernelJson, "waitTime", item.waitTime, allocator);
        JsonUtil::AddMember(kernelJson, "blockDim", item.blockDim, allocator);
        JsonUtil::AddMember(kernelJson, "inputShapes", item.inputShapes, allocator);
        JsonUtil::AddMember(kernelJson, "inputDataTypes", item.inputDataTypes, allocator);
        JsonUtil::AddMember(kernelJson, "inputFormats", item.inputFormats, allocator);
        JsonUtil::AddMember(kernelJson, "outputShapes", item.outputShapes, allocator);
        JsonUtil::AddMember(kernelJson, "outputDataTypes", item.outputDataTypes, allocator);
        JsonUtil::AddMember(kernelJson, "outputFormats", item.outputFormats, allocator);
        details.PushBack(kernelJson, allocator);
    }
    JsonUtil::AddMember(body, "kernelDetails", details, allocator);
    json_t accCoreList(kArrayType);
    for (const auto& item : response.body.acceleratorCoreList) {
        json_t itemJson;
        itemJson.SetString(item.c_str(), item.length(), allocator);
        accCoreList.PushBack(itemJson, allocator);
    }

    JsonUtil::AddMember(body, "acceleratorCoreList", accCoreList, allocator);
    JsonUtil::AddMember(body, "count", response.body.count, allocator);
    JsonUtil::AddMember(body, "pageSize", response.body.pageSize, allocator);
    JsonUtil::AddMember(body, "currentPage", response.body.currentPage, allocator);

    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<OneKernelResponse>(const OneKernelResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "depth", response.body.depth, allocator);
    JsonUtil::AddMember(body, "threadId", response.body.threadId, allocator);
    JsonUtil::AddMember(body, "pid", response.body.pid, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}
#pragma endregion

#pragma region <<Event to json>>
template <typename EVENT> std::optional<document_t> ToEventJson(const EVENT &event)
{
    return std::nullopt;
}

json_t UnitTrackToJson(const UnitTrack &unitTrack, RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
{
    json_t json(kObjectType);
    JsonUtil::AddMember(json, "type", unitTrack.type, allocator);
    json_t metadata(kObjectType);
    JsonUtil::AddMember(metadata, "cardId", unitTrack.metaData.cardId, allocator);
    JsonUtil::AddMember(metadata, "processId", unitTrack.metaData.processId, allocator);
    JsonUtil::AddMember(metadata, "processName", unitTrack.metaData.processName, allocator);
    JsonUtil::AddMember(metadata, "label", unitTrack.metaData.label, allocator);
    JsonUtil::AddMember(metadata, "threadId", unitTrack.metaData.threadId, allocator);
    JsonUtil::AddMember(metadata, "threadName", unitTrack.metaData.threadName, allocator);
    JsonUtil::AddMember(metadata, "maxDepth", unitTrack.metaData.maxDepth, allocator);
    json_t dataType(kArrayType);
    for (const auto &type : unitTrack.metaData.dataType) {
        dataType.PushBack(json_t().SetString(type.c_str(), type.length(), allocator), allocator);
    }
    JsonUtil::AddMember(metadata, "dataType", dataType, allocator);
    json_t children(kArrayType);
    for (const auto &track : unitTrack.children) {
        children.PushBack(UnitTrackToJson(*track, allocator), allocator);
    }
    JsonUtil::AddMember(json, "children", children, allocator);
    JsonUtil::AddMember(json, "metadata", metadata, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToEventJson<ParseSuccessEvent>(const ParseSuccessEvent &event)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetEventJsonBaseInfo(event, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "maxTimeStamp", event.body.maxTimeStamp, allocator);
    JsonUtil::AddMember(body, "startTimeUpdated", event.body.startTimeUpdated, allocator);
    json_t unit(kObjectType);
    JsonUtil::AddMember(unit, "type", event.body.unit.type, allocator);
    json_t metadata(kObjectType);
    JsonUtil::AddMember(metadata, "cardId", event.body.unit.metadata.cardId, allocator);
    JsonUtil::AddMember(unit, "metadata", metadata, allocator);
    json_t children(kArrayType);
    for (const auto &track : event.body.unit.children) {
        children.PushBack(UnitTrackToJson(*track, allocator), allocator);
    }
    JsonUtil::AddMember(unit, "children", children, allocator);
    JsonUtil::AddMember(body, "unit", unit, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToEventJson<ParseFailEvent>(const ParseFailEvent &event)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetEventJsonBaseInfo(event, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "rankId", event.body.rankId, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToEventJson<ParseClusterCompletedEvent>(const ParseClusterCompletedEvent &event)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetEventJsonBaseInfo(event, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "parseResult", event.body.parseResult, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToEventJson<ParseMemoryCompletedEvent>(const ParseMemoryCompletedEvent &event)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetEventJsonBaseInfo(event, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "isCluster", event.isCluster, allocator);
    json_t memoryList(kArrayType);
    for (const MemorySuccess &memory: event.memoryResult) {
        json_t memoryJson(kObjectType);
        JsonUtil::AddMember(memoryJson, "rankId", memory.rankId, allocator);
        JsonUtil::AddMember(memoryJson, "hasMemory", memory.hasFile and memory.parseSuccess, allocator);
        memoryList.PushBack(memoryJson, allocator);
    }

    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic