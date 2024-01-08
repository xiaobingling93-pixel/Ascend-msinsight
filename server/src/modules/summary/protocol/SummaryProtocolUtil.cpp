/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "ServerLog.h"
#include "JsonUtil.h"
#include "ProtocolEnumUtil.h"
#include "SummaryProtocol.h"
#include "SummaryProtocolUtil.h"

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

template <> std::optional<document_t> ToResponseJson<SummaryTopRankResponse>(const SummaryTopRankResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t stepList(kArrayType);
    for (const auto &item: response.body.stepList) {
        json_t itemJson;
        itemJson.SetString(item.c_str(), item.length(), allocator);
        stepList.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "stepList", stepList, allocator);

    json_t rankList(kArrayType);
    for (const auto &item: response.body.rankList) {
        json_t itemJson;
        itemJson.SetString(item.c_str(), item.length(), allocator);
        rankList.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "rankList", rankList, allocator);

    JsonUtil::AddMember(body, "rankCount", response.body.rankCount, allocator);
    JsonUtil::AddMember(body, "dataSize", response.body.dataSize, allocator);
    JsonUtil::AddMember(body, "collectStartTime", response.body.collectStartTime, allocator);
    JsonUtil::AddMember(body, "filePath", response.body.filePath, allocator);
    JsonUtil::AddMember(body, "collectDuration", response.body.collectDuration, allocator);
    JsonUtil::AddMember(body, "stepNum", response.body.stepNum, allocator);

    json_t summaryList(kArrayType);
    for (const SummaryDto& summaryDto : response.body.summaryList) {
        json_t summaryJson(kObjectType);
        JsonUtil::AddMember(summaryJson, "rankId", summaryDto.rankId, allocator);
        JsonUtil::AddMember(summaryJson, "totalTime", summaryDto.totalTime, allocator);
        JsonUtil::AddMember(summaryJson, "computingTime", summaryDto.computingTime, allocator);
        JsonUtil::AddMember(summaryJson, "communicationOverLappedTime",
                            summaryDto.communicationOverLappedTime, allocator);
        JsonUtil::AddMember(summaryJson, "communicationNotOverLappedTime",
                            summaryDto.communicationNotOverLappedTime, allocator);
        JsonUtil::AddMember(summaryJson, "freeTime", summaryDto.freeTime, allocator);
        summaryList.PushBack(summaryJson, allocator);
    }
    JsonUtil::AddMember(body, "summaryList", summaryList, allocator);

    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<SummaryStatisticsResponse>(const SummaryStatisticsResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);

    json_t summaryList(kArrayType);
    for (const SummaryStatisticsItem &action : response.body.summaryStatisticsItemList) {
        json_t summaryJson(kObjectType);
        JsonUtil::AddMember(summaryJson, "acceleratorCore", action.acceleratorCore, allocator);
        JsonUtil::AddMember(summaryJson, "overlapType", action.overlapType, allocator);
        JsonUtil::AddMember(summaryJson, "duration", action.duration, allocator);
        JsonUtil::AddMember(summaryJson, "utilization", action.utilization, allocator);
        summaryList.PushBack(summaryJson, allocator);
    }
    JsonUtil::AddMember(body, "summaryStatisticsItemList", summaryList, allocator);

    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<ComputeDetailResponse>(const ComputeDetailResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);

    JsonUtil::AddMember(body, "totalNum", response.totalNum, allocator);
    json_t detail(kArrayType);
    for (const ComputeDetail &item : response.computeDetails) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "name", item.name, allocator);
        JsonUtil::AddMember(itemJson, "type", item.type, allocator);
        JsonUtil::AddMember(itemJson, "startTime", item.startTime, allocator);
        JsonUtil::AddMember(itemJson, "duration", item.duration, allocator);
        JsonUtil::AddMember(itemJson, "waitTime", item.waitTime, allocator);
        JsonUtil::AddMember(itemJson, "blockDim", item.blockDim, allocator);
        JsonUtil::AddMember(itemJson, "inputShapes", item.inputShapes, allocator);
        JsonUtil::AddMember(itemJson, "inputDataTypes", item.inputDataTypes, allocator);
        JsonUtil::AddMember(itemJson, "inputFormats", item.inputFormats, allocator);
        JsonUtil::AddMember(itemJson, "outputShapes", item.outputShapes, allocator);
        JsonUtil::AddMember(itemJson, "outputDataTypes", item.outputDataTypes, allocator);
        JsonUtil::AddMember(itemJson, "outputFormats", item.outputFormats, allocator);
        detail.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "computeDetails", detail, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<PipelineStepResponse>(const PipelineStepResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);

    json_t data(kArrayType);
    for (const std::string &step : response.body.stepList) {
        json_t itemJson;
        itemJson.SetString(step.c_str(), step.length(), allocator);
        data.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<PipelineStageResponse>(const PipelineStageResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t data(kArrayType);
    for (const std::string &stage : response.body.stageList) {
        json_t itemJson;
        itemJson.SetString(stage.c_str(), stage.length(), allocator);
        data.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<PipelineStageTimeResponse>(const PipelineStageTimeResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t details(kArrayType);
    for (const BubbleDetail &item : response.body.bubbleDetails) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "stageId", item.stageOrRankId, allocator);
        JsonUtil::AddMember(itemJson, "stageTime", item.stageTime, allocator);
        JsonUtil::AddMember(itemJson, "bubbleTime", item.bubbleTime, allocator);
        details.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "stageAndBubbleTimes", details, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<PipelineRankTimeResponse>(const PipelineRankTimeResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t details(kArrayType);
    for (const BubbleDetail &item : response.body.bubbleDetails) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "stageId", item.stageOrRankId, allocator);
        JsonUtil::AddMember(itemJson, "stageTime", item.stageTime, allocator);
        JsonUtil::AddMember(itemJson, "bubbleTime", item.bubbleTime, allocator);
        details.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "stageAndBubbleTimes", details, allocator);
    return std::move(json);
}
template <>
std::optional<document_t> ToResponseJson<CommunicationDetailResponse>(const CommunicationDetailResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);

    JsonUtil::AddMember(body, "totalNum", response.totalNum, allocator);
    json_t details(kArrayType);

    for (const CommunicationDetail &detail : response.commDetails) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "name",  detail.name, allocator);
        JsonUtil::AddMember(itemJson, "type",  detail.type, allocator);
        JsonUtil::AddMember(itemJson, "startTime",  detail.startTime, allocator);
        JsonUtil::AddMember(itemJson, "duration",  detail.duration, allocator);
        JsonUtil::AddMember(itemJson, "waitTime",  detail.waitTime, allocator);
        details.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "stageAndBubbleTimes", details, allocator);
    return std::move(json);
}

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic