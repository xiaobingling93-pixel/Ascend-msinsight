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

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic