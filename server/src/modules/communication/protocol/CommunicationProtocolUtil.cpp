/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "ServerLog.h"
#include "JsonUtil.h"
#include "ProtocolEnumUtil.h"
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
    json_t details(kArrayType);
    for (const OperatorItem& operatorItem : response.body.allOperators) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "operatorName", operatorItem.operatorName, allocator);
        JsonUtil::AddMember(itemJson, "elapseTime", operatorItem.elapseTime, allocator);
        JsonUtil::AddMember(itemJson, "transitTime", operatorItem.transitTime, allocator);
        JsonUtil::AddMember(itemJson, "synchronizationTime", operatorItem.synchronizationTime, allocator);
        JsonUtil::AddMember(itemJson, "waitTime", operatorItem.waitTime, allocator);
        JsonUtil::AddMember(itemJson, "idleTime", operatorItem.idleTime, allocator);
        JsonUtil::AddMember(itemJson, "synchronizationTimeRatio", operatorItem.synchronizationTimeRatio, allocator);
        JsonUtil::AddMember(itemJson, "waitTimeRatio", operatorItem.waitTimeRatio, allocator);
        details.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "allOperators", details, allocator);
    JsonUtil::AddMember(body, "count", response.body.count, allocator);
    JsonUtil::AddMember(body, "pageSize", response.body.pageSize, allocator);
    JsonUtil::AddMember(body, "currentPage", response.body.currentPage, allocator);
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

template <> std::optional<document_t> ToResponseJson<CommunicatorGroupResponse>(const CommunicatorGroupResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "defaultPPSize", response.body.defaultPPSize, allocator);
    json_t ppGroups(kArrayType);
    for (const auto& item : response.body.ppGroups) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "name", item.name, allocator);
        JsonUtil::AddMember(itemJson, "value", item.value, allocator);
        JsonUtil::AddArrayMember(itemJson, "ranks", item.ranks, allocator);
        ppGroups.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "ppGroups", ppGroups, allocator);
    json_t tpOrDpGroups(kArrayType);
    for (auto item : response.body.tpOrDpGroups) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "name", item.name, allocator);
        JsonUtil::AddMember(itemJson, "value", item.value, allocator);
        JsonUtil::AddArrayMember(itemJson, "ranks", item.ranks, allocator);
        tpOrDpGroups.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "tpOrDpGroups", tpOrDpGroups, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<IterationsOrRanksResponse>(const IterationsOrRanksResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t rankIds(kArrayType);
    for (const IterationsOrRanksObject& ranksObject : response.body) {
        json_t item(kObjectType);
        item.SetString(ranksObject.iterationOrRankId.c_str(), ranksObject.iterationOrRankId.length(), allocator);
        rankIds.PushBack(item, allocator);
    }
    JsonUtil::AddMember(body, "iterationOrRankId", rankIds, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<OperatorNamesResponse>(const OperatorNamesResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t operators(kArrayType);
    for (const OperatorNamesObject& object : response.body) {
        json_t item(kObjectType);
        item.SetString(object.operatorName.c_str(), object.operatorName.length(), allocator);
        operators.PushBack(item, allocator);
    }
    JsonUtil::AddMember(body, "operatorName", operators, allocator);
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
    for (const Duration& duration : response.body) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "rankId", duration.rankId, allocator);
        JsonUtil::AddMember(itemJson, "elapseTime", duration.elapseTime, allocator);
        JsonUtil::AddMember(itemJson, "transitTime", duration.transitTime, allocator);
        JsonUtil::AddMember(itemJson, "synchronizationTime", duration.synchronizationTime, allocator);
        JsonUtil::AddMember(itemJson, "waitTime", duration.waitTime, allocator);
        JsonUtil::AddMember(itemJson, "idleTime", duration.idleTime, allocator);
        JsonUtil::AddMember(itemJson, "synchronizationTimeRatio", duration.synchronizationTimeRatio, allocator);
        JsonUtil::AddMember(itemJson, "waitTimeRatio", duration.waitTimeRatio, allocator);
        items.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "items", items, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<RanksResponse>(const RanksResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t rankIds(kArrayType);
    for (const IterationsOrRanksObject& object : response.body) {
        json_t item(kObjectType);
        item.SetString(object.iterationOrRankId.c_str(), object.iterationOrRankId.length(), allocator);
        rankIds.PushBack(item, allocator);
    }
    JsonUtil::AddMember(body, "iterationOrRankId", rankIds, allocator);
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
        json_t item(kObjectType);
        item.SetString(action.c_str(), action.length(), allocator);
        data.PushBack(item, allocator);
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
    json_t matrixLists(kArrayType);
    for (const MatrixList& matrixList : response.body.matrixList) {
        json_t itemJson(kObjectType);
        JsonUtil::AddMember(itemJson, "srcRank", matrixList.srcRank, allocator);
        JsonUtil::AddMember(itemJson, "dstRank", matrixList.dstRank, allocator);
        JsonUtil::AddMember(itemJson, "transportType", matrixList.transportType, allocator);
        JsonUtil::AddMember(itemJson, "transitSize", matrixList.transitSize, allocator);
        JsonUtil::AddMember(itemJson, "transitTime", matrixList.transitTime, allocator);
        JsonUtil::AddMember(itemJson, "bandwidth", matrixList.bandwidth, allocator);
        JsonUtil::AddMember(itemJson, "transitSize", matrixList.transitSize, allocator);
        matrixLists.PushBack(itemJson, allocator);
    }
    JsonUtil::AddMember(body, "matrixList", matrixLists, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic