/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "EventUtil.h"

namespace Dic {
namespace Scene {
namespace Core {
using namespace Server;
EventUtil::EventUtil()
{
    Register();
}

EventUtil::~EventUtil()
{
    UnRegister();
}

void EventUtil::Register()
{
    jsonToEventFactory.emplace("M", ToMetaDataEvent);
    jsonToEventFactory.emplace("X", ToSliceEvent);
    jsonToEventFactory.emplace("s", ToFlowEvent);
    jsonToEventFactory.emplace("f", ToFlowEvent);
}

void EventUtil::UnRegister()
{
    jsonToEventFactory.clear();
}

std::string EventUtil::Type(const json_t &json)
{
    if (json.contains("ph") && json["ph"].is_string()) {
        return json["ph"];
    }
    return "";
}

std::unique_ptr<Event> EventUtil::FromJson(const json_t &json)
{
    const std::string type = Type(json);
    if (type.empty()) {
        ServerLog::Warn("Can't find the type. json:", json.dump());
        return nullptr;
    }
    std::optional<EventUtil::JsonToEventFunc> func = GetJsonToEventFunc(type);
    if (!func.has_value()) {
        return nullptr;
    }
    return func.value()(json);
}

std::optional<EventUtil::JsonToEventFunc> EventUtil::GetJsonToEventFunc(const std::string &type)
{
    if (jsonToEventFactory.count(type) == 0) {
        ServerLog::Warn("The json to event function is not find. type:", type);
        return std::nullopt;
    }
    return jsonToEventFactory[type];
}

std::unique_ptr<Event> EventUtil::ToSliceEvent(const json_t &json)
{
    std::unique_ptr<Slice> event = std::make_unique<Slice>();
    event->type = Type(json);
    event->ts = JsonUtil::GetDouble(json, "ts");
    event->dur = JsonUtil::GetDouble(json, "dur");
    event->name = JsonUtil::GetString(json, "name");
    event->tid = JsonUtil::GetInteger(json, "tid");
    event->pid = JsonUtil::GetString(json, "pid");
    event->cat = JsonUtil::GetOptionalString(json, "cat");
    event->args = JsonUtil::GetOptionalString(json, "args");
    return event;
}

std::unique_ptr<Event> EventUtil::ToMetaDataEvent(const json_t &json)
{
    std::unique_ptr<MetaData> event = std::make_unique<MetaData>();
    event->type = Type(json);
    event->name = JsonUtil::GetString(json, "name");
    event->tid = JsonUtil::GetInteger(json, "tid");
    event->pid = JsonUtil::GetString(json, "pid");
    if (json.contains("args")) {
        event->args.name = JsonUtil::GetString(json["args"], "name");
        event->args.labels = JsonUtil::GetString(json["args"], "labels");
        event->args.sortIndex = JsonUtil::GetInteger(json["args"], "sort_index");
    }
    return event;
}

std::unique_ptr<Event> EventUtil::ToFlowEvent(const json_t &json)
{
    std::unique_ptr<Flow> event = std::make_unique<Flow>();
    event->type = Type(json);
    event->ts = JsonUtil::GetDouble(json, "ts");
    event->tid = JsonUtil::GetInteger(json, "tid");
    event->pid = JsonUtil::GetString(json, "pid");
    auto flowId = JsonUtil::GetOptionalString(json, "id");
    event->flowId = flowId.has_value() ? flowId.value() : "";
    event->name = JsonUtil::GetString(json, "name");
    event->cat = JsonUtil::GetOptionalString(json, "cat");
    return event;
}

} // end of namespace Core
} // end of namespace Scene
} // end of namespace Dic