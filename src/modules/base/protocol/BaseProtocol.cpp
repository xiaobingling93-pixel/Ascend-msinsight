/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "BaseProtocol.h"
#include "ProtocolDefs.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
void BaseProtocol::Register()
{
    std::lock_guard<std::mutex> lock(mutex);
    RegisterJsonToRequestFuncs();
    RegisterResponseToJsonFuncs();
    RegisterEventToJsonFuncs();
}

void BaseProtocol::UnRegister()
{
    std::lock_guard<std::mutex> lock(mutex);
    jsonToReqFactory.clear();
    resToJsonFactory.clear();
    eventToJsonFactory.clear();
}

std::unique_ptr<Request> BaseProtocol::FromJson(const json_t &requestJson, std::string &error)
{
    if (!IsRequest(requestJson)) {
        ServerLog::Warn("json type is not request.", requestJson.dump());
        return nullptr;
    }
    const std::string command = Command(requestJson);
    std::optional<BaseProtocol::JsonToRequestFunc> func = GetJsonToRequestFunc(command);
    if (!func.has_value()) {
        return nullptr;
    }
    return func.value()(requestJson, error);
}

std::optional<json_t> BaseProtocol::ToJson(const Response &response, std::string &error)
{
    std::string command = response.command;
    std::optional<BaseProtocol::ResponseToJsonFunc> func = GetResponseToJsonFunc(command);
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

std::optional<json_t> BaseProtocol::ToJson(const Event &event, std::string &error)
{
    std::string eventString = event.event;
    std::optional<BaseProtocol::EventToJsonFunc> func = GetEventToJsonFunc(eventString);
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

bool BaseProtocol::IsRequest(const json_t &jsonRequest)
{
    return (jsonRequest.contains("type")) && (jsonRequest["type"] == REQUEST_NAME);
}

std::string BaseProtocol::Command(const json_t &jsonRequest)
{
    if (jsonRequest.contains("command") && jsonRequest["command"].is_string()) {
        return jsonRequest["command"].get<std::string>();
    }
    return "";
}

std::optional<BaseProtocol::JsonToRequestFunc> BaseProtocol::GetJsonToRequestFunc(const std::string &command)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (jsonToReqFactory.count(command) == 0) {
        ServerLog::Warn("The json to request function is not find. command:", command);
        return std::nullopt;
    }
    return jsonToReqFactory[command];
}

std::optional<BaseProtocol::ResponseToJsonFunc> BaseProtocol::GetResponseToJsonFunc(const std::string &command)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (resToJsonFactory.count(command) == 0) {
        ServerLog::Warn("The response to json function is not find. command:", command);
        return std::nullopt;
    }
    return resToJsonFactory[command];
}

std::optional<BaseProtocol::EventToJsonFunc> BaseProtocol::GetEventToJsonFunc(const std::string &event)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (eventToJsonFactory.count(event) == 0) {
        return std::nullopt;
    }
    return eventToJsonFactory[event];
}
}
}