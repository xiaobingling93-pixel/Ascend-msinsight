/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "ProtocolEnumUtil.h"
#include "ProtocolUtil.h"
#include "GlobalProtocolUtil.h"
#include "GlobalProtocol.h"

namespace Dic {
namespace Protocol {
void GlobalProtocol::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_RES_TOKEN_CREATE, ToTokenCreateRequest);
    jsonToReqFactory.emplace(REQ_RES_TOKEN_DESTROY, ToTokenDestroyRequest);
    jsonToReqFactory.emplace(REQ_RES_TOKEN_CHECK, ToTokenCheckRequest);
    jsonToReqFactory.emplace(REQ_RES_CONFIG_GET, ToConfigGetRequest);
    jsonToReqFactory.emplace(REQ_RES_CONFIG_SET, ToConfigSetRequest);
}

void GlobalProtocol::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_RES_TOKEN_CREATE, ToTokenCreateResponseJson);
    resToJsonFactory.emplace(REQ_RES_TOKEN_DESTROY, ToTokenDestroyResponseJson);
    resToJsonFactory.emplace(REQ_RES_TOKEN_CHECK, ToTokenCheckResponseJson);
    resToJsonFactory.emplace(REQ_RES_CONFIG_GET, ToConfigGetResponseJson);
    resToJsonFactory.emplace(REQ_RES_CONFIG_SET, ToConfigSetResponseJson);
}

void GlobalProtocol::RegisterEventToJsonFuncs()
{

}

#pragma region <<Json To Request>>
std::unique_ptr<Request> GlobalProtocol::ToTokenCreateRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<TokenCreateRequest> reqPtr = std::make_unique<TokenCreateRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    if (JsonUtil::IsJsonKeyValid(json, "params")) {
        JsonUtil::SetByJsonKeyValue(reqPtr->params.deadTime, json["params"], "deadTime");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.parentToken, json["params"], "parentToken");
    }
    return reqPtr;
}

std::unique_ptr<Request> GlobalProtocol::ToTokenDestroyRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<TokenDestroyRequest> reqPtr = std::make_unique<TokenDestroyRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.destroyToken, json["params"], "destroyToken");
    return reqPtr;
}

std::unique_ptr<Request> GlobalProtocol::ToTokenCheckRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<TokenCheckRequest> reqPtr = std::make_unique<TokenCheckRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.checkedToken, json["params"], "checkedToken");
    return reqPtr;
}

std::unique_ptr<Request> GlobalProtocol::ToConfigGetRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<ConfigGetRequest> reqPtr = std::make_unique<ConfigGetRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.moduleMask, json["params"], "moduleMask");
    return reqPtr;
}

std::unique_ptr<Request> GlobalProtocol::ToConfigSetRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<ConfigSetRequest> reqPtr = std::make_unique<ConfigSetRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    if (!JsonUtil::IsJsonKeyValid(json["params"], "config")) {
        error = "Failed to get config object, command is: " + reqPtr->command;
        return nullptr;
    }
    if (JsonUtil::IsJsonKeyValid(json["params"]["config"], "global")) {
        GlobalConfig config;
        ProtocolUtil::SetGlobalConfigStruct(json["params"]["config"]["global"], config);
        reqPtr->params.globalConfig = config;
    }
    return reqPtr;
}
#pragma endregion

#pragma region <<Response To Json>>
std::optional<json_t> GlobalProtocol::ToTokenCreateResponseJson(const Response &response)
{
    return ToResponseJson<TokenCreateResponse>(dynamic_cast<const TokenCreateResponse &>(response));
}

std::optional<json_t> GlobalProtocol::ToTokenDestroyResponseJson(const Response &response)
{
    return ToResponseJson<TokenDestroyResponse>(dynamic_cast<const TokenDestroyResponse &>(response));
}

std::optional<json_t> GlobalProtocol::ToTokenCheckResponseJson(const Response &response)
{
    return ToResponseJson<TokenCheckResponse>(dynamic_cast<const TokenCheckResponse &>(response));
}

std::optional<json_t> GlobalProtocol::ToConfigGetResponseJson(const Response &response)
{
    return ToResponseJson<ConfigGetResponse>(dynamic_cast<const ConfigGetResponse &>(response));
}

std::optional<json_t> GlobalProtocol::ToConfigSetResponseJson(const Response &response)
{
    return ToResponseJson<ConfigSetResponse>(dynamic_cast<const ConfigSetResponse &>(response));
}

#pragma endregion

#pragma region <<Event To Json>>
#pragma endregion
} // namespace Protocol
} // namespace Dic
