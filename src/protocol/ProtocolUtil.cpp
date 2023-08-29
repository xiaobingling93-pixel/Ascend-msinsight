/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "JsonUtil.h"
#include "ProtocolDefs.h"
#include "ProtocolUtil.h"

namespace Dic {
namespace Protocol {
void ProtocolUtil::SetRequestJsonBaseInfo(const Request &request, json_t &json)
{
    json["type"] = REQUEST_NAME;
    json["id"] = request.id;
    json["command"] = request.command;
    json["moduleName"] = ENUM_TO_STR(request.moduleName).value();
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
    request.type = STR_TO_ENUM<Dic::Protocol::ProtocolMessage::Type>(json["type"]).value();
    request.moduleName = STR_TO_ENUM<Dic::Protocol::ModuleType>(json["moduleName"]).value();
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
    json["moduleName"] = ENUM_TO_STR(response.moduleName).value();
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
    response.type = STR_TO_ENUM<Dic::Protocol::ProtocolMessage::Type>(json["type"]).value();
    response.moduleName = STR_TO_ENUM<Dic::Protocol::ModuleType>(json["moduleName"]).value();
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
    json["moduleName"] = ENUM_TO_STR(event.moduleName).value();
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
    event.type = STR_TO_ENUM<Dic::Protocol::ProtocolMessage::Type>(json["type"]).value();
    event.moduleName = STR_TO_ENUM<Dic::Protocol::ModuleType>(json["moduleName"]).value();
    JsonUtil::SetByJsonKeyValue<std::string>(event.token, json["body"], "token");
    if (json.contains("resultCallbackId")) {
        event.resultCallbackId = json["resultCallbackId"];
    }
    return true;
}
} // end of namespace Protocol
} // end of namespace Dic