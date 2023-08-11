/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "ProtocolEnumUtil.h"
#include "ProtocolUtil.h"
#include "ResponseUtil.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
#pragma region << template_functions>>
template <typename RESPONSE> std::optional<json_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("ToResponseJson is not implemented. command:", response.command);
    return std::nullopt;
}

template <> std::optional<json_t> ToResponseJson<TokenCreateResponse>(const TokenCreateResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["createTime"] = response.body.createTime;
    if (response.body.parentToken.has_value() && !response.body.parentToken.value().empty()) {
        json["body"]["parentToken"] = response.body.parentToken.value();
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<TokenDestroyResponse>(const TokenDestroyResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["destroyTime"] = response.body.destroyTime;
    json["body"]["destroyToken"] = response.body.destroyToken;
    return json;
}

template <> std::optional<json_t> ToResponseJson<TokenCheckResponse>(const TokenCheckResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["checkedToken"] = response.body.checkedToken;
    json["body"]["deadTime"] = response.body.deadTime;
    json["body"]["createTime"] = response.body.createTime;
    return json;
}

template <> std::optional<json_t> ToResponseJson<ConfigGetResponse>(const ConfigGetResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    if (response.body.globalConfig.has_value()) {
        json["body"]["config"]["global"] = json_t::object();
        ProtocolUtil::SetGlobalConfigJson(response.body.globalConfig.value(), json["body"]["config"]["global"]);
    }
    if (response.body.harmonyConfig.has_value()) {
        json["body"]["config"]["harmony"] = json_t::object();
        ProtocolUtil::SetHarmonyConfigJson(response.body.harmonyConfig.value(), json["body"]["config"]["harmony"]);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<ConfigSetResponse>(const ConfigSetResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["configSetTime"] = response.body.configSetTime;
    json["body"]["isAlertMsg"] = response.body.isAlertMsg;
    return json;
}

template <> std::optional<json_t> ToResponseJson<HdcDeviceListResponse>(const HdcDeviceListResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["deviceList"] = json_t::array();
    for (const Device& device : response.body.deviceList) {
        json_t devJson = json_t::object();
        devJson["deviceKey"] = device.deviceKey;
        devJson["cpuAbi"] = device.cpuAbi;
        devJson["apiVersion"] = device.apiVersion;
        devJson["productModel"] = device.productModel;
        devJson["deviceType"] = device.deviceType;
        devJson["softwareVersion"] = device.softwareVersion;
        devJson["status"] = ENUM_TO_STR<DeviceStatus>(device.status).value();
        devJson["connectType"] = ENUM_TO_STR<DeviceConnectType>(device.connectType).value();
        devJson["productBrand"] = device.productBrand;
        json["body"]["deviceList"].push_back(devJson);
    }
    return json;
}

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic