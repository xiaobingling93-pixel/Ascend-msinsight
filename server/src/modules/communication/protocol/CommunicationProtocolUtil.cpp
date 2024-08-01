/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "pch.h"
#include "CommunicationProtocol.h"
#include "CommunicationProtocolUtil.h"

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

template <> std::optional<document_t> ToResponseJson<OperatorDetailsResponse>(const OperatorDetailsResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t allOperators(kArrayType);
    for (const OperatorItem& operatorItem : response.body.allOperators) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "operatorName", operatorItem.operatorName, allocator);
        JsonUtil::AddMember(itemJson, "startTime", operatorItem.startTime, allocator);
        JsonUtil::AddMember(itemJson, "elapseTime", operatorItem.elapseTime, allocator);
        JsonUtil::AddMember(itemJson, "transitTime", operatorItem.transitTime, allocator);
        JsonUtil::AddMember(itemJson, "synchronizationTime", operatorItem.synchronizationTime, allocator);
        JsonUtil::AddMember(itemJson, "waitTime", operatorItem.waitTime, allocator);
        JsonUtil::AddMember(itemJson, "idleTime", operatorItem.idleTime, allocator);
        JsonUtil::AddMember(itemJson, "synchronizationTimeRatio",
                            operatorItem.synchronizationTimeRatio, allocator);
        JsonUtil::AddMember(itemJson, "waitTimeRatio", operatorItem.waitTimeRatio, allocator);
        JsonUtil::AddMember(itemJson, "sdmaBw", operatorItem.sdmaBw, allocator);
        JsonUtil::AddMember(itemJson, "rdmaBw", operatorItem.rdmaBw, allocator);
        allOperators.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "currentPage", response.body.currentPage, allocator);
    JsonUtil::AddMember(body, "pageSize", response.body.pageSize, allocator);
    JsonUtil::AddMember(body, "count", response.body.count, allocator);
    JsonUtil::AddMember(body, "allOperators", allOperators, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<DistributionResponse>(const DistributionResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "distributionData", response.body.distributionData, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<BandwidthDataResponse>(const BandwidthDataResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t items(kArrayType);
    for (const BandwidthDataItem& bandwidthDataItem : response.body.items) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "transportType", bandwidthDataItem.transportType, allocator);
        JsonUtil::AddMember(itemJson, "transitSize", bandwidthDataItem.transitSize, allocator);
        JsonUtil::AddMember(itemJson, "transitTime", bandwidthDataItem.transitTime, allocator);
        JsonUtil::AddMember(itemJson, "bandwidth", bandwidthDataItem.bandwidth, allocator);
        JsonUtil::AddMember(itemJson, "largePacketRatio", bandwidthDataItem.largePacketRatio, allocator);
        items.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "items", items, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <>
std::optional<document_t> ToResponseJson<CommunicatorGroupResponse>(const CommunicatorGroupResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);

    json_t body(response.body, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <>
std::optional<document_t> ToResponseJson<IterationsOrRanksResponse>(const IterationsOrRanksResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t iterationOrRankId(kArrayType);
    for (const IterationsOrRanksObject& ranksObject : response.body) {
        iterationOrRankId.PushBack(json_t().SetString(ranksObject.iterationOrRankId.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(body, "iterationOrRankId", iterationOrRankId, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}
template <> std::optional<document_t> ToResponseJson<OperatorNamesResponse>(const OperatorNamesResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t operatorName(kArrayType);
    for (const OperatorNamesObject& object : response.body) {
        operatorName.PushBack(json_t().SetString(object.operatorName.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(body, "operatorName", operatorName, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <>
std::optional<document_t> ToResponseJson<MatrixSortOpNamesResponse>(const MatrixSortOpNamesResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t operatorName(kArrayType);
    for (const OperatorNamesObject& object : response.body) {
        operatorName.PushBack(json_t().SetString(object.operatorName.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(body, "operatorName", operatorName, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<DurationResponse>(const DurationResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t items(kArrayType);
    for (const Duration& duration : response.body.durationList) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "rankId", duration.rankId, allocator);
        JsonUtil::AddMember(itemJson, "startTime", duration.startTime, allocator);
        JsonUtil::AddMember(itemJson, "elapseTime", duration.elapseTime, allocator);
        JsonUtil::AddMember(itemJson, "transitTime", duration.transitTime, allocator);
        JsonUtil::AddMember(itemJson, "synchronizationTime", duration.synchronizationTime, allocator);
        JsonUtil::AddMember(itemJson, "waitTime", duration.waitTime, allocator);
        JsonUtil::AddMember(itemJson, "idleTime", duration.idleTime, allocator);
        JsonUtil::AddMember(itemJson, "synchronizationTimeRatio", duration.synchronizationTimeRatio, allocator);
        JsonUtil::AddMember(itemJson, "waitTimeRatio", duration.waitTimeRatio, allocator);
        JsonUtil::AddMember(itemJson, "sdmaBw", duration.sdmaBw, allocator);
        JsonUtil::AddMember(itemJson, "rdmaBw", duration.rdmaBw, allocator);
        items.PushBack(itemJson, allocator);
    }
    json_t adviceJson(kArrayType);
    for (const auto& item : response.body.bwStatistics) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "type", item.type, allocator);
        JsonUtil::AddMember(itemJson, "max", item.maxBw, allocator);
        JsonUtil::AddMember(itemJson, "min", item.minBw, allocator);
        JsonUtil::AddMember(itemJson, "avg", item.avgBw, allocator);
        JsonUtil::AddMember(itemJson, "diff", item.diffBw, allocator);
        JsonUtil::AddMember(itemJson, "time", item.allTime, allocator);
        adviceJson.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "items", items, allocator);
    JsonUtil::AddMember(body, "advice", adviceJson, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<OperatorListsResponse>(const OperatorListsResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t dataArray(kArrayType);
    for (size_t i = 0; i < response.body.rankLists.size(); ++i) {
        json_t oneRankJson(kObjectType);
        JsonUtil::AddMember(oneRankJson, "rankId", response.body.rankLists[i], allocator);
        json_t opListJson(kArrayType);
        for (auto item : response.body.opLists[i]) {
            json_t opJson(kObjectType);
            JsonUtil::AddMember(opJson, "operatorName", item.operatorName, allocator);
            JsonUtil::AddMember(opJson, "startTime", item.startTime, allocator);
            JsonUtil::AddMember(opJson, "duration", item.elapseTime, allocator);
            opListJson.PushBack(opJson, allocator);
        }
        JsonUtil::AddMember(oneRankJson, "lists", opListJson, allocator);
        dataArray.PushBack(oneRankJson, allocator);
    }
    JsonUtil::AddMember(body, "data", dataArray, allocator);
    JsonUtil::AddMember(body, "minTime", response.body.minTime, allocator);
    JsonUtil::AddMember(body, "maxTime", response.body.maxTime, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<RanksResponse>(const RanksResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t iterationOrRankId(kArrayType);
    for (const IterationsOrRanksObject& object : response.body) {
        iterationOrRankId.PushBack(json_t().SetString(object.iterationOrRankId.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(body, "iterationOrRankId", iterationOrRankId, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<MatrixGroupResponse>(const MatrixGroupResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t data(kArrayType);
    for (const std::string &action : response.body.groupList) {
        data.PushBack(json_t().SetString(action.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<MatrixListResponse>(const MatrixListResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t matrixList(kArrayType);
    for (const MatrixList& matrix : response.body.matrixList) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "srcRank", matrix.srcRank, allocator);
        JsonUtil::AddMember(itemJson, "dstRank", matrix.dstRank, allocator);
        JsonUtil::AddMember(itemJson, "transportType", matrix.transportType, allocator);
        JsonUtil::AddMember(itemJson, "transitSize", matrix.transitSize, allocator);
        JsonUtil::AddMember(itemJson, "transitTime", matrix.transitTime, allocator);
        JsonUtil::AddMember(itemJson, "bandwidth", matrix.bandwidth, allocator);
        JsonUtil::AddMember(itemJson, "transitSize", matrix.transitSize, allocator);
        JsonUtil::AddMember(itemJson, "opName", matrix.opName, allocator);
        matrixList.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "matrixList", matrixList, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic