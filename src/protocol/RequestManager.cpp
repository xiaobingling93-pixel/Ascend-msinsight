/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: Debug Adapter Protocol request manager implementation
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "ProtocolEnumUtil.h"
#include "ProtocolUtil.h"
#include "RequestManager.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
RequestManager::RequestManager()
{
    Register();
}

RequestManager::~RequestManager()
{
    UnRegister();
}

void RequestManager::Register()
{
    std::lock_guard<std::mutex> lock(mutex);
    RegisterJsonToRequestFuncs();
}

void RequestManager::RegisterJsonToRequestFuncs()
{
    // global
    jsonToReqFactory.emplace(REQ_RES_TOKEN_CREATE, ToTokenCreateRequest);
    jsonToReqFactory.emplace(REQ_RES_TOKEN_DESTROY, ToTokenDestroyRequest);
    jsonToReqFactory.emplace(REQ_RES_TOKEN_CHECK, ToTokenCheckRequest);
    jsonToReqFactory.emplace(REQ_RES_CONFIG_GET, ToConfigGetRequest);
    jsonToReqFactory.emplace(REQ_RES_CONFIG_SET, ToConfigSetRequest);

    jsonToReqFactory.emplace(REQ_RES_HDC_DEVICE_LIST, ToHdcDeviceListRequest);
}

void RequestManager::UnRegister()
{
    std::lock_guard<std::mutex> lock(mutex);
    jsonToReqFactory.clear();
}

const std::optional<RequestManager::JsonToRequestFunc> RequestManager::GetJsonToRequestFunc(const std::string &command)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (jsonToReqFactory.count(command) == 0) {
        ServerLog::Warn("The json to request function is not find. command:", command);
        return std::nullopt;
    }
    return jsonToReqFactory[command];
}

bool RequestManager::IsRequest(const json_t &jsonRequest)
{
    return (jsonRequest.contains("type")) && (jsonRequest["type"] == REQUEST_NAME);
}

const std::string RequestManager::Command(const json_t &jsonRequest) const
{
    if (jsonRequest.contains("command") && jsonRequest["command"].is_string()) {
        return jsonRequest["command"].get<std::string>();
    }
    return "";
}

/*
 * 将json数据转为Request
 *
 * requestJson：json数据
 * error：转换过程中的错误信息
 * @return 转换后的结果
 */
const std::unique_ptr<Request> RequestManager::FromJson(const json_t &requestJson, std::string &error)
{
    if (!IsRequest(requestJson)) {
        error = "json type is not request.";
        return nullptr;
    }
    const std::string command = Command(requestJson);
    std::optional<RequestManager::JsonToRequestFunc> func = GetJsonToRequestFunc(command);
    if (!func.has_value()) {
        return nullptr;
    }
    return func.value()(requestJson, error);
}

/*
 * 将json格式的string转为Request
 *
 * requestStr：json格式的string
 * error：转换过程中的错误信息
 * @return 转换后的结果
 */
const std::unique_ptr<Request> RequestManager::FromJson(const std::string &requestStr, std::string &error)
{
    json_t requestJson;
    try {
        requestJson = json_t::parse(requestStr);
    } catch (json_t::parse_error &) {
        return nullptr;
    }
    return FromJson(requestJson, error);
}

#pragma region << Json To Request>>

// global
std::unique_ptr<Request> RequestManager::ToTokenCreateRequest(const json_t &json, std::string &error)
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

std::unique_ptr<Request> RequestManager::ToTokenDestroyRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<TokenDestroyRequest> reqPtr = std::make_unique<TokenDestroyRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.destroyToken, json["params"], "destroyToken");
    return reqPtr;
}

std::unique_ptr<Request> RequestManager::ToTokenCheckRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<TokenCheckRequest> reqPtr = std::make_unique<TokenCheckRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.checkedToken, json["params"], "checkedToken");
    return reqPtr;
}

std::unique_ptr<Request> RequestManager::ToConfigGetRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<ConfigGetRequest> reqPtr = std::make_unique<ConfigGetRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.sceneMask, json["params"], "sceneMask");
    return reqPtr;
}

std::unique_ptr<Request> RequestManager::ToConfigSetRequest(const json_t &json, std::string &error)
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
    if (JsonUtil::IsJsonKeyValid(json["params"]["config"], "harmony")) {
        HarmonyConfig config;
        ProtocolUtil::SetHarmonyConfigStruct(json["params"]["config"]["harmony"], config);
        reqPtr->params.harmonyConfig = config;
    }
    return reqPtr;
}

std::unique_ptr<Request> RequestManager::ToHdcDeviceListRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<HdcDeviceListRequest> reqPtr = std::make_unique<HdcDeviceListRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.timeout, json["params"], "timeout");
    return reqPtr;
}

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic