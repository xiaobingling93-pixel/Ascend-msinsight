/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "pch.h"
#include "SummaryProtocol.h"
#include "SummaryProtocolUtil.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
using namespace rapidjson;
#pragma region <<Response to json>>
template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("Function to response json is not implemented. command:", response.command);
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
        stepList.PushBack(json_t().SetString(item.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(body, "stepList", stepList, allocator);
    json_t rankList(kArrayType);
    for (const auto &item: response.body.rankList) {
        rankList.PushBack(json_t().SetString(item.c_str(), allocator), allocator);
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
        json_t summaryDtoJson(kObjectType);
        JsonUtil::AddMember(summaryDtoJson, "rankId", summaryDto.rankId, allocator);
        JsonUtil::AddMember(summaryDtoJson, "totalTime", summaryDto.totalTime, allocator);
        JsonUtil::AddMember(summaryDtoJson, "computingTime", summaryDto.computingTime, allocator);
        JsonUtil::AddMember(summaryDtoJson, "communicationOverLappedTime",
                            summaryDto.communicationOverLappedTime, allocator);
        JsonUtil::AddMember(summaryDtoJson, "communicationNotOverLappedTime",
                            summaryDto.communicationNotOverLappedTime, allocator);
        JsonUtil::AddMember(summaryDtoJson, "freeTime", summaryDto.freeTime, allocator);
        JsonUtil::AddMember(summaryDtoJson, "prepareTime", summaryDto.prepareTime, allocator);
        summaryList.PushBack(summaryDtoJson, allocator);
    }
    json_t adviceJson(kObjectType);
    TraceStatistic stat = response.body.traceStatistic;
    int digit = 2;
    JsonUtil::AddMember(adviceJson, "compute", NumberUtil::DoubleReservedNDigits(stat.computeDiff, digit), allocator);
    JsonUtil::AddMember(adviceJson, "communication",
                        NumberUtil::DoubleReservedNDigits(stat.communicationDiff, digit), allocator);
    JsonUtil::AddMember(adviceJson, "free", NumberUtil::DoubleReservedNDigits(stat.freeDiff, digit), allocator);

    JsonUtil::AddMember(body, "summaryList", summaryList, allocator);
    JsonUtil::AddMember(body, "advice", adviceJson, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <>
std::optional<document_t> ToResponseJson<SummaryStatisticsResponse>(const SummaryStatisticsResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t summaryStatisticsItemList(kArrayType);
    for (const SummaryStatisticsItem &action : response.body.summaryStatisticsItemList) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "acceleratorCore", action.acceleratorCore, allocator);
        JsonUtil::AddMember(itemJson, "overlapType", action.overlapType, allocator);
        JsonUtil::AddMember(itemJson, "duration", action.duration, allocator);
        JsonUtil::AddMember(itemJson, "utilization", action.utilization, allocator);
        summaryStatisticsItemList.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "summaryStatisticsItemList", summaryStatisticsItemList, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<ComputeDetailResponse>(const ComputeDetailResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t computeDetails(kArrayType);
    for (const ComputeDetail &action : response.computeDetails) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "name", action.name, allocator);
        JsonUtil::AddMember(itemJson, "type", action.type, allocator);
        JsonUtil::AddMember(itemJson, "startTime", action.startTime, allocator);
        JsonUtil::AddMember(itemJson, "duration", action.duration, allocator);
        JsonUtil::AddMember(itemJson, "waitTime", action.waitTime, allocator);
        JsonUtil::AddMember(itemJson, "blockDim", action.blockDim, allocator);
        JsonUtil::AddMember(itemJson, "inputShapes", action.inputShapes, allocator);
        JsonUtil::AddMember(itemJson, "inputDataTypes", action.inputDataTypes, allocator);
        JsonUtil::AddMember(itemJson, "inputFormats", action.inputFormats, allocator);
        JsonUtil::AddMember(itemJson, "outputShapes", action.outputShapes, allocator);
        JsonUtil::AddMember(itemJson, "outputDataTypes", action.outputDataTypes, allocator);
        JsonUtil::AddMember(itemJson, "outputFormats", action.outputFormats, allocator);
        computeDetails.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "computeDetails", computeDetails, allocator);
    JsonUtil::AddMember(body, "totalNum", response.totalNum, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<PipelineStepResponse>(const PipelineStepResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t data(kArrayType);
    for (const std::string &action : response.body.stepList) {
        data.PushBack(json_t().SetString(action.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<PipelineStageResponse>(const PipelineStageResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t data(kArrayType);
    for (const std::string &action : response.body.stageList) {
        data.PushBack(json_t().SetString(action.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <>
std::optional<document_t> ToResponseJson<PipelineStageTimeResponse>(const PipelineStageTimeResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t stageAndBubbleTimes(kArrayType);
    for (const BubbleDetail &action : response.body.bubbleDetails) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "stageId", action.stageOrRankId, allocator);
        JsonUtil::AddMember(itemJson, "stageTime", action.stageTime, allocator);
        JsonUtil::AddMember(itemJson, "bubbleTime", action.bubbleTime, allocator);
        stageAndBubbleTimes.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "stageAndBubbleTimes", stageAndBubbleTimes, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<PipelineRankTimeResponse>(const PipelineRankTimeResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t stageAndBubbleTimes(kArrayType);
    for (const BubbleDetail &action : response.body.bubbleDetails) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "rankId", action.stageOrRankId, allocator);
        JsonUtil::AddMember(itemJson, "stageTime", action.stageTime, allocator);
        JsonUtil::AddMember(itemJson, "bubbleTime", action.bubbleTime, allocator);
        stageAndBubbleTimes.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "stageAndBubbleTimes", stageAndBubbleTimes, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <>
std::optional<document_t> ToResponseJson<CommunicationDetailResponse>(const CommunicationDetailResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t communicationDetails(kArrayType);
    for (const CommunicationDetail &detail : response.commDetails) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "name", detail.name, allocator);
        JsonUtil::AddMember(itemJson, "type", detail.type, allocator);
        JsonUtil::AddMember(itemJson, "startTime", detail.startTime, allocator);
        JsonUtil::AddMember(itemJson, "duration", detail.duration, allocator);
        JsonUtil::AddMember(itemJson, "waitTime", detail.waitTime, allocator);
        communicationDetails.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "communicationDetails", communicationDetails, allocator);
    JsonUtil::AddMember(body, "totalNum", response.totalNum, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <>
std::optional<document_t> ToResponseJson<QueryParallelStrategyResponse>(const QueryParallelStrategyResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, KEY_ALGORITHM, response.config.algorithm, allocator);
    JsonUtil::AddMember(body, KEY_LEVEL, response.level, allocator);
    JsonUtil::AddMember(body, KEY_TP_SIZE, response.config.tpSize, allocator);
    JsonUtil::AddMember(body, KEY_PP_SIZE, response.config.ppSize, allocator);
    JsonUtil::AddMember(body, KEY_DP_SIZE, response.config.dpSize, allocator);
    JsonUtil::AddMember(json, KEY_BODY, body, allocator);
    return std::move(json);
}

template <>
std::optional<document_t> ToResponseJson<SetParallelStrategyResponse>(const SetParallelStrategyResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, KEY_RESULT, response.result, allocator);
    JsonUtil::AddMember(body, KEY_MSG, response.msg, allocator);
    JsonUtil::AddMember(json, KEY_BODY, body, allocator);
    return std::move(json);
}
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic