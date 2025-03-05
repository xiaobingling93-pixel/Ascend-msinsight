/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "ClusterDef.h"
#include "pch.h"
#include "SummaryProtocol.h"
#include "SummaryProtocolUtil.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
using namespace Dic::Module;
using namespace rapidjson;
#pragma region <<Response to json>>
template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("Function to response json is not implemented. command:", response.command);
    return std::nullopt;
}

std::optional<document_t> BaseInfoToJson(const SummaryBaseInfo &baseInfo, Document::AllocatorType &allocator)
{
    document_t json(kObjectType);
    json_t stepList(kArrayType);
    for (const auto &item: baseInfo.stepList) {
        stepList.PushBack(json_t().SetString(item.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(json, "stepList", stepList, allocator);
    json_t rankList(kArrayType);
    for (const auto &item: baseInfo.rankList) {
        rankList.PushBack(json_t().SetString(item.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(json, "rankList", rankList, allocator);
    JsonUtil::AddMember(json, "rankCount", baseInfo.rankCount, allocator);
    JsonUtil::AddMember(json, "dataSize", baseInfo.dataSize, allocator);
    JsonUtil::AddMember(json, "collectStartTime", baseInfo.collectStartTime, allocator);
    JsonUtil::AddMember(json, "filePath", baseInfo.filePath, allocator);
    JsonUtil::AddMember(json, "collectDuration", baseInfo.collectDuration, allocator);
    JsonUtil::AddMember(json, "stepNum", baseInfo.stepNum, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<SummaryTopRankResponse>(const SummaryTopRankResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t stepList(kArrayType);
    json_t baseInfo(kObjectType);
    auto compareData = BaseInfoToJson(response.body.baseInfo.compare, allocator);
    JsonUtil::AddMember(baseInfo, "compare", compareData, allocator);
    auto baselineData = BaseInfoToJson(response.body.baseInfo.baseline, allocator);
    JsonUtil::AddMember(baseInfo, "baseline", baselineData, allocator);
    JsonUtil::AddMember(body, "baseInfo", baseInfo, allocator);
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
    JsonUtil::AddMember(body, KEY_CP_SIZE, response.config.cpSize, allocator);
    JsonUtil::AddMember(body, KEY_EP_SIZE, response.config.epSize, allocator);
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

template <>
std::optional<document_t> ToResponseJson<PipelineFwdBwdTimelineResponse>(const PipelineFwdBwdTimelineResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "minTime", response.body.minTime, allocator);
    JsonUtil::AddMember(body, "maxTime", response.body.maxTime, allocator);
    json_t rankDetailList(kArrayType);
    if (response.body.rankDataList.empty()) {
        JsonUtil::AddMember(body, "rankList", rankDetailList, allocator);
        JsonUtil::AddMember(json, KEY_BODY, body, allocator);
        return std::move(json);
    }

    for (const PipelineFwdBwdTimelineByRank &rankItem : response.body.rankDataList) {
        json_t rankArrayJson(kObjectType);
        JsonUtil::AddMember(rankArrayJson, "rank", rankItem.rankId, allocator);
        json_t componentsDetailList(kArrayType);
        for (const PipelineFwdBwdTimelineByComponent &componentItem : rankItem.componentDataList) {
            json_t componentJson(kObjectType);
            JsonUtil::AddMember(componentJson, "component", componentItem.component, allocator);
            json_t tracesDetailList(kArrayType);
            for (const Protocol::ThreadTraces &traceItem : componentItem.traceList) {
                json_t traceJson(kObjectType);
                JsonUtil::AddMember(traceJson, "name", traceItem.name, allocator);
                JsonUtil::AddMember(traceJson, "start", traceItem.startTime, allocator);
                JsonUtil::AddMember(traceJson, "duration", traceItem.duration, allocator);
                JsonUtil::AddMember(traceJson, "pid", traceItem.pid, allocator);
                JsonUtil::AddMember(traceJson, "tid", traceItem.threadId, allocator);
                JsonUtil::AddMember(traceJson, "id", traceItem.id, allocator);
                JsonUtil::AddMember(traceJson, "cname", traceItem.cname, allocator);
                tracesDetailList.PushBack(traceJson, allocator);
            }
            JsonUtil::AddMember(componentJson, "traceList", tracesDetailList, allocator);
            componentsDetailList.PushBack(componentJson, allocator);
        }
        JsonUtil::AddMember(rankArrayJson, "componentList", componentsDetailList, allocator);
        rankDetailList.PushBack(rankArrayJson, allocator);
    }
    JsonUtil::AddMember(body, "rankList", rankDetailList, allocator);
    JsonUtil::AddMember(json, KEY_BODY, body, allocator);
    return std::move(json);
}

void GetArrangementsJson(const ParallelismArrangementResponse& response, document_t& json, json_t& body)
{
    json_t arrangements(kArrayType);
    auto &allocator = json.GetAllocator();
    for (const auto& arrangement : response.arrangeData.arrangements) {
        json_t arrangementJson(kObjectType);
        JsonUtil::AddMember(arrangementJson, "index", arrangement.index, allocator);
        JsonUtil::AddMember(arrangementJson, "name", arrangement.name, allocator);
        json_t positionJson(kObjectType);
        JsonUtil::AddMember(positionJson, "x", arrangement.position.x, allocator);
        JsonUtil::AddMember(positionJson, "y", arrangement.position.y, allocator);
        JsonUtil::AddMember(arrangementJson, "position", positionJson, allocator);
        json_t attributeJson(kObjectType);
        for (const auto& indexAttr : arrangement.indexAttributes) {
            JsonUtil::AddMember(attributeJson, indexAttr.first, indexAttr.second, allocator);
        }
        JsonUtil::AddMember(arrangementJson, "attribute", attributeJson, allocator);
        JsonUtil::AddMember(arrangementJson, "ranks", arrangement.ranks, allocator);
        arrangements.PushBack(arrangementJson, allocator);
    }
    JsonUtil::AddMember(body, "arrangements", arrangements, allocator);
}

template <>
std::optional<document_t> ToResponseJson<ParallelismArrangementResponse>(const ParallelismArrangementResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "size", response.arrangeData.size, allocator);
    GetArrangementsJson(response, json, body);
    json_t indicators(kArrayType);
    for (const auto& indicator : response.arrangeData.indicators) {
        json_t indicatorJson(kObjectType);
        JsonUtil::AddMember(indicatorJson, "key", indicator.key, allocator);
        JsonUtil::AddMember(indicatorJson, "name", indicator.name, allocator);
        JsonUtil::AddMember(indicatorJson, "renderHeatMap", indicator.renderHeatMap, allocator);
        JsonUtil::AddMember(indicatorJson, "renderChart", indicator.renderChart, allocator);
        JsonUtil::AddMember(indicatorJson, "visible", indicator.visible, allocator);
        JsonUtil::AddMember(indicatorJson, "chart", indicator.chart, allocator);
        JsonUtil::AddMember(indicatorJson, "stack", indicator.stack, allocator);
        JsonUtil::AddMember(indicatorJson, "yAxisType", indicator.yAxisType, allocator);
        indicators.PushBack(indicatorJson, allocator);
    }
    JsonUtil::AddMember(body, "indicators", indicators, allocator);
    json_t connections(kArrayType);
    for (const auto& connection : response.arrangeData.connections) {
        json_t connectionJson(kObjectType);
        JsonUtil::AddMember(connectionJson, "type", connection.type, allocator);
        json_t listJson(kArrayType);
        for (const auto& index : connection.indexes) {
            uint32_t tmpIdx = index;
            listJson.PushBack(json_t().SetInt(tmpIdx), allocator);
        }
        JsonUtil::AddMember(connectionJson, "list", listJson, allocator);
        JsonUtil::AddMember(connectionJson, "group", connection.communicationGroups, allocator);
        connections.PushBack(connectionJson, allocator);
    }
    JsonUtil::AddMember(body, "connections", connections, allocator);
    JsonUtil::AddMember(json, KEY_BODY, body, allocator);
    return std::move(json);
}

std::optional<document_t> IndicatorsInfoToJson(const std::unordered_map<std::string, double> &indicators,
                                               Document::AllocatorType &allocator)
{
    document_t dataJson(kObjectType);
    for (const auto& indicator :indicators) {
        JsonUtil::AddMember(dataJson, indicator.first, indicator.second, allocator);
    }
    return std::move(dataJson);
}

template <>
std::optional<document_t> ToResponseJson<ParallelismPerformanceResponse>(const ParallelismPerformanceResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t performance(kArrayType);
    for (const auto& data : response.indicatorData.performanceData) {
        json_t dataJson(kObjectType);
        JsonUtil::AddMember(dataJson, "index", data.index, allocator);
        json_t indicatorsJson(kObjectType);
        auto compare = IndicatorsInfoToJson(data.indicators.compare, allocator);
        JsonUtil::AddMember(indicatorsJson, "compare", compare, allocator);
        auto baseline = IndicatorsInfoToJson(data.indicators.baseline, allocator);
        JsonUtil::AddMember(indicatorsJson, "baseline", baseline, allocator);
        auto diff = IndicatorsInfoToJson(data.indicators.diff, allocator);
        JsonUtil::AddMember(indicatorsJson, "diff", diff, allocator);
        JsonUtil::AddMember(dataJson, "indicators", indicatorsJson, allocator);

        json_t commIndicatorsJson(kObjectType);
        auto commCompare = IndicatorsInfoToJson(data.commTimeIndicator.compare, allocator);
        JsonUtil::AddMember(commIndicatorsJson, "compare", commCompare, allocator);
        auto commBaseline = IndicatorsInfoToJson(data.commTimeIndicator.baseline, allocator);
        JsonUtil::AddMember(commIndicatorsJson, "baseline", commBaseline, allocator);
        auto commDiff = IndicatorsInfoToJson(data.commTimeIndicator.diff, allocator);
        JsonUtil::AddMember(commIndicatorsJson, "diff", commDiff, allocator);
        JsonUtil::AddMember(dataJson, "commTimeIndicator", commIndicatorsJson, allocator);
        performance.PushBack(dataJson, allocator);
    }
    JsonUtil::AddMember(body, "performance", performance, allocator);
    JsonUtil::AddMember(body, "advice", response.indicatorData.advices, allocator);
    JsonUtil::AddMember(json, KEY_BODY, body, allocator);
    return std::move(json);
}
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic