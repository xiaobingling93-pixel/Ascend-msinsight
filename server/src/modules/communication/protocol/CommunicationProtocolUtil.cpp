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
#pragma region <<Response to json>>
template <typename RESPONSE> std::optional<json_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("ToResponseJson is not implemented. command:", response.command);
    return std::nullopt;
}

template <> std::optional<json_t> ToResponseJson<OperatorDetailsResponse>(const OperatorDetailsResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["allOperators"] = json_t::array();
    for (const OperatorItem& operatorItem : response.body.allOperators) {
        json_t itemJson = json_t::object();
        itemJson["operatorName"] = operatorItem.operatorName;
        itemJson["elapseTime"] = operatorItem.elapseTime;
        itemJson["transitTime"] = operatorItem.transitTime;
        itemJson["synchronizationTime"] = operatorItem.synchronizationTime;
        itemJson["waitTime"] = operatorItem.waitTime;
        itemJson["idleTime"] = operatorItem.idleTime;
        itemJson["synchronizationTimeRatio"] = operatorItem.synchronizationTimeRatio;
        itemJson["waitTimeRatio"] = operatorItem.waitTimeRatio;
        json["body"]["allOperators"].emplace_back(itemJson);
    }
    json["body"]["count"] = response.body.count;
    json["body"]["pageSize"] = response.body.pageSize;
    json["body"]["currentPage"] = response.body.currentPage;
    return json;
}

template <> std::optional<json_t> ToResponseJson<DistributionResponse>(const DistributionResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["distributionData"] = response.body.distributionData;
    return json;
}

template <> std::optional<json_t> ToResponseJson<BandwidthDataResponse>(const BandwidthDataResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["items"] = json_t::array();
    for (const BandwidthDataItem& bandwidthDataItem : response.body.items) {
        json_t itemJson = json_t::object();
        itemJson["transportType"] = bandwidthDataItem.transportType;
        itemJson["transitSize"] = bandwidthDataItem.transitSize;
        itemJson["transitTime"] = bandwidthDataItem.transitTime;
        itemJson["bandwidth"] = bandwidthDataItem.bandwidth;
        itemJson["largePacketRatio"] = bandwidthDataItem.largePacketRatio;
        json["body"]["items"].emplace_back(itemJson);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<CommunicatorGroupResponse>(const CommunicatorGroupResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);

    json["body"]["defaultPPSize"] = response.body.defaultPPSize;
    json["body"]["ppGroups"] = json_t::array();
    for (auto item : response.body.ppGroups) {
        json_t itemJson = json_t::object();
        itemJson["name"] = item.name;
        itemJson["value"] = item.value;
        itemJson["ranks"] = item.ranks;
        json["body"]["ppGroups"].emplace_back(itemJson);
    }
    for (auto item : response.body.tpOrDpGroups) {
        json_t itemJson = json_t::object();
        itemJson["name"] = item.name;
        itemJson["value"] = item.value;
        itemJson["ranks"] = item.ranks;
        json["body"]["tpOrDpGroups"].emplace_back(itemJson);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<IterationsOrRanksResponse>(const IterationsOrRanksResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["iterationOrRankId"] = json_t::array();
    for (const IterationsOrRanksObject& ranksObject : response.body) {
        json["body"]["iterationOrRankId"].emplace_back(ranksObject.iterationOrRankId);
    }
    return json;
}
template <> std::optional<json_t> ToResponseJson<OperatorNamesResponse>(const OperatorNamesResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["operatorName"] = json_t::array();
    for (const OperatorNamesObject& object : response.body) {
        json["body"]["operatorName"].emplace_back(object.operatorName);
    }
    return json;
}
template <> std::optional<json_t> ToResponseJson<DurationResponse>(const DurationResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["items"] = json_t::array();
    for (const Duration& duration : response.body) {
        json_t itemJson = json_t::object();
        itemJson["rankId"] = duration.rankId;
        itemJson["elapseTime"] = duration.elapseTime;
        itemJson["transitTime"] = duration.transitTime;
        itemJson["synchronizationTime"] = duration.synchronizationTime;
        itemJson["waitTime"] = duration.waitTime;
        itemJson["idleTime"] = duration.idleTime;
        itemJson["synchronizationTimeRatio"] = duration.synchronizationTimeRatio;
        itemJson["waitTimeRatio"] = duration.waitTimeRatio;
        json["body"]["items"].emplace_back(itemJson);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<RanksResponse>(const RanksResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["iterationOrRankId"] = json_t::array();
    for (const IterationsOrRanksObject& object : response.body) {
        json["body"]["iterationOrRankId"].emplace_back(object.iterationOrRankId);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<MatrixGroupResponse>(const MatrixGroupResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["data"] = json_t::array();
    for (const std::string &action : response.body.groupList) {
        json["body"]["data"].emplace_back(action);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<MatrixListResponse>(const MatrixListResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["matrixList"] = json_t::array();
    for (const MatrixList& matrixList : response.body.matrixList) {
        json_t itemJson = json_t::object();
        itemJson["srcRank"] = matrixList.srcRank;
        itemJson["dstRank"] = matrixList.dstRank;
        itemJson["transportType"] = matrixList.transportType;
        itemJson["transitTime"] = matrixList.transitTime;
        itemJson["bandwidth"] = matrixList.bandwidth;
        json["body"]["matrixList"].emplace_back(itemJson);
    }
    return json;
}

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic