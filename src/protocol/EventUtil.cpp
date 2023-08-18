/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "ProtocolEnumUtil.h"
#include "JsonUtil.h"
#include "ProtocolUtil.h"
#include "ProtocolEvent.h"
#include "EventUtil.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Protocol;
#pragma region << template_functions>>
template <typename EVENT> std::optional<json_t> ToEventJson(const EVENT &event)
{
    return std::nullopt;
}

template <> std::optional<json_t> ToEventJson<DeviceChangedEvent>(const DeviceChangedEvent &event)
{
    json_t json;
    ProtocolUtil::SetEventJsonBaseInfo(event, json);
    json["body"]["device"]["deviceKey"] = event.body.device.deviceKey;
    json["body"]["device"]["cpuAbi"] = event.body.device.cpuAbi;
    json["body"]["device"]["apiVersion"] = event.body.device.apiVersion;
    json["body"]["device"]["productModel"] = event.body.device.productModel;
    json["body"]["device"]["deviceType"] = event.body.device.deviceType;
    json["body"]["device"]["softwareVersion"] = event.body.device.softwareVersion;
    json["body"]["device"]["status"] = ENUM_TO_STR<DeviceStatus>(event.body.device.status).value();
    json["body"]["device"]["connectType"] = ENUM_TO_STR<DeviceConnectType>(event.body.device.connectType).value();
    json["body"]["device"]["productBrand"] = event.body.device.productBrand;
    return json;
}

json_t UnitTrackToJson(const UnitTrack &unitTrack) {
    json_t json;
    json["type"] = unitTrack.type;
    json["metadata"]["cardId"] = unitTrack.metaData.cardId;
    json["metadata"]["processId"] = unitTrack.metaData.processId;
    json["metadata"]["processName"] = unitTrack.metaData.processName;
    json["metadata"]["label"] = unitTrack.metaData.label;
    json["metadata"]["threadId"] = unitTrack.metaData.threadId;
    json["metadata"]["threadName"] = unitTrack.metaData.threadName;
    json["metadata"]["maxDepth"] = unitTrack.metaData.maxDepth;
    for (const auto &track : unitTrack.children) {
        json["children"].emplace_back(UnitTrackToJson(*track));
    }
    return json;
}

template <> std::optional<json_t> ToEventJson<ParseSuccessEvent>(const ParseSuccessEvent &event)
{
    json_t json;
    ProtocolUtil::SetEventJsonBaseInfo(event, json);
    json["body"]["maxTimeStamp"] = event.body.maxTimeStamp;
    json["body"]["startTimeUpdated"] = event.body.startTimeUpdated;
    json["body"]["unit"]["type"] = event.body.unit.type;
    json["body"]["unit"]["metadata"]["cardId"] = event.body.unit.metadata.cardId;
    for (const auto &track : event.body.unit.children) {
        json["body"]["unit"]["children"].emplace_back(UnitTrackToJson(*track));
    }
    return json;
}

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic