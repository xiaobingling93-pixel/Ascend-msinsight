/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "Protocol.h"
#include "ProtocolUtil.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
void ProtocolUtil::Register()
{
    std::lock_guard<std::mutex> lock(mutex);
    RegisterJsonToRequestFuncs();
    RegisterResponseToJsonFuncs();
    RegisterEventToJsonFuncs();
}

void ProtocolUtil::UnRegister()
{
    std::lock_guard<std::mutex> lock(mutex);
    jsonToReqFactory.clear();
    resToJsonFactory.clear();
    eventToJsonFactory.clear();
}

std::unique_ptr<Request> ProtocolUtil::FromJson(const json_t &requestJson, std::string &error)
{
    if (!IsRequest(requestJson)) {
        ServerLog::Warn("json type is not request.", requestJson.dump());
        return nullptr;
    }
    const std::string command = Command(requestJson);
    std::optional<ProtocolUtil::JsonToRequestFunc> func = GetJsonToRequestFunc(command);
    if (!func.has_value()) {
        return nullptr;
    }
    return func.value()(requestJson, error);
}

std::optional<json_t> ProtocolUtil::ToJson(const Response &response, std::string &error)
{
    std::string command = response.command;
    std::optional<ProtocolUtil::ResponseToJsonFunc> func = GetResponseToJsonFunc(command);
    if (!func.has_value()) {
        error = "Failed to find response target function, token = " + response.token + ", command = " + command;
        return std::nullopt;
    }
    try {
        return func.value()(response);
    } catch (std::exception &e) {
        error = "Failed to convert response to json, command = " + command;
        return std::nullopt;
    }
}

std::optional<json_t> ProtocolUtil::ToJson(const Event &event, std::string &error)
{
    std::string eventString = event.event;
    std::optional<ProtocolUtil::EventToJsonFunc> func = GetEventToJsonFunc(eventString);
    if (!func.has_value()) {
        error = "Failed to find event target function, token = " + event.token + ", event = " + eventString +
            ", module:" + std::to_string(static_cast<int>(event.moduleName));
        return std::nullopt;
    }
    try {
        return func.value()(event);
    } catch (std::exception &e) {
        error = "Failed to convert event to json, event = " + eventString;
        return std::nullopt;
    }
}

void ProtocolUtil::SetRequestJsonBaseInfo(const Request &request, json_t &json)
{
    json["type"] = REQUEST_NAME;
    json["id"] = request.id;
    json["command"] = request.command;
    auto moduleName = ENUM_TO_STR(request.moduleName);
    json["moduleName"] = moduleName.has_value() ? moduleName.value() : MODULE_UNKNOWN;
    json["params"]["token"] = request.token;
    if (request.resultCallbackId.has_value()) {
        json["resultCallbackId"] = request.resultCallbackId.value();
    }
}

bool ProtocolUtil::SetRequestBaseInfo(Request &request, const json_t &json)
{
    if (!JsonUtil::IsJsonKeyValid(json, "id") || !JsonUtil::IsJsonKeyValid(json, "type") ||
        !JsonUtil::IsJsonKeyValid(json, "command")) {
        return false;
    }
    if (!JsonUtil::IsJsonKeyValid(json, "params") || !JsonUtil::IsJsonKeyValid(json, "moduleName")) {
        return false;
    }
    request.id = json["id"];
    request.command = json["command"];
    auto type = STR_TO_ENUM<Dic::Protocol::ProtocolMessage::Type>(json["type"]);
    request.type = type.has_value() ? type.value() : ProtocolMessage::Type::NONE;
    auto moduleName = STR_TO_ENUM<Dic::Protocol::ModuleType>(json["moduleName"]);
    request.moduleName = moduleName.has_value() ? moduleName.value() : ModuleType::UNKNOWN;
    JsonUtil::SetByJsonKeyValue<std::string>(request.token, json["params"], "token");
    if (json.contains("resultCallbackId")) {
        request.resultCallbackId = json["resultCallbackId"];
    }
    return true;
}

void ProtocolUtil::SetResponseJsonBaseInfo(const Response &response, json_t &json)
{
    json["type"] = RESPONSE_NAME;
    json["id"] = response.id;
    json["requestId"] = response.requestId;
    json["result"] = response.result;
    json["command"] = response.command;
    auto moduleName = ENUM_TO_STR(response.moduleName);
    json["moduleName"] = moduleName.has_value() ? moduleName.value() : MODULE_UNKNOWN;
    if (response.error.has_value()) {
        json["error"]["code"] = response.error.value().code;
        json["error"]["message"] = response.error.value().message;
    }
    json["body"]["token"] = response.token;
    if (response.resultCallbackId.has_value()) {
        json["resultCallbackId"] = response.resultCallbackId.value();
    }
}

bool ProtocolUtil::SetResponseBaseInfo(Response &response, const json_t &json)
{
    if (!JsonUtil::IsJsonKeyValid(json, "result") || !JsonUtil::IsJsonKeyValid(json, "id") ||
        !JsonUtil::IsJsonKeyValid(json, "type") || !JsonUtil::IsJsonKeyValid(json, "command") ||
        !JsonUtil::IsJsonKeyValid(json, "requestId")) {
        return false;
    }
    if (!JsonUtil::IsJsonKeyValid(json, "body") || !JsonUtil::IsJsonKeyValid(json, "moduleName")) {
        return false;
    }
    response.result = json["result"];
    response.id = json["id"];
    response.requestId = json["requestId"];
    response.command = json["command"];
    auto type = STR_TO_ENUM<Dic::Protocol::ProtocolMessage::Type>(json["type"]);
    response.type = type.has_value() ? type.value() : ProtocolMessage::Type::NONE;
    auto moduleName = STR_TO_ENUM<Dic::Protocol::ModuleType>(json["moduleName"]);
    response.moduleName = moduleName.has_value() ? moduleName.value() : ModuleType::UNKNOWN;
    JsonUtil::SetByJsonKeyValue<std::string>(response.token, json["body"], "token");
    if (json.contains("resultCallbackId")) {
        response.resultCallbackId = json["resultCallbackId"];
    }
    return true;
}

void ProtocolUtil::SetEventJsonBaseInfo(const Event &event, json_t &json)
{
    json["type"] = EVENT_NAME;
    json["id"] = event.id;
    json["event"] = event.event;
    auto moduleName = ENUM_TO_STR(event.moduleName);
    json["moduleName"] = moduleName.has_value() ? moduleName.value() : MODULE_UNKNOWN;
    json["body"]["token"] = event.token;
    if (event.resultCallbackId.has_value()) {
        json["resultCallbackId"] = event.resultCallbackId.value();
    }
}

bool ProtocolUtil::SetEventBaseInfo(Event &event, const json_t &json)
{
    if (!JsonUtil::IsJsonKeyValid(json, "id") || !JsonUtil::IsJsonKeyValid(json, "type") ||
        !JsonUtil::IsJsonKeyValid(json, "event")) {
        return false;
    }
    if (!JsonUtil::IsJsonKeyValid(json, "body") || !JsonUtil::IsJsonKeyValid(json, "moduleName")) {
        return false;
    }
    event.id = json["id"];
    event.event = json["event"];
    auto type = STR_TO_ENUM<Dic::Protocol::ProtocolMessage::Type>(json["type"]);
    event.type = type.has_value() ? type.value() : ProtocolMessage::Type::NONE;
    auto moduleName = STR_TO_ENUM<Dic::Protocol::ModuleType>(json["moduleName"]);
    event.moduleName = moduleName.has_value() ? moduleName.value() : ModuleType::UNKNOWN;
    JsonUtil::SetByJsonKeyValue<std::string>(event.token, json["body"], "token");
    if (json.contains("resultCallbackId")) {
        event.resultCallbackId = json["resultCallbackId"];
    }
    return true;
}

bool ProtocolUtil::IsRequest(const json_t &jsonRequest)
{
    return (jsonRequest.contains("type")) && (jsonRequest["type"] == REQUEST_NAME);
}

std::string ProtocolUtil::Command(const json_t &jsonRequest)
{
    if (jsonRequest.contains("command") && jsonRequest["command"].is_string()) {
        return jsonRequest["command"].get<std::string>();
    }
    return "";
}

std::optional<ProtocolUtil::JsonToRequestFunc> ProtocolUtil::GetJsonToRequestFunc(const std::string &command)
{
    ServerLog::Error(command);
    ServerLog::Error(jsonToReqFactory.count(command));
    std::lock_guard<std::mutex> lock(mutex);
    if (jsonToReqFactory.count(command) == 0) {
        ServerLog::Warn("The json to request function is not find. command:", command);
        return std::nullopt;
    }
    return jsonToReqFactory[command];
}

std::optional<ProtocolUtil::ResponseToJsonFunc> ProtocolUtil::GetResponseToJsonFunc(const std::string &command)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (resToJsonFactory.count(command) == 0) {
        ServerLog::Warn("The response to json function is not find. command:", command);
        return std::nullopt;
    }
    return resToJsonFactory[command];
}

std::optional<ProtocolUtil::EventToJsonFunc> ProtocolUtil::GetEventToJsonFunc(const std::string &event)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (eventToJsonFactory.count(event) == 0) {
        return std::nullopt;
    }
    return eventToJsonFactory[event];
}
}
}