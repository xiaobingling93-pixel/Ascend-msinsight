/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "EventUtil.h"

namespace Dic {
namespace Module {
namespace Timeline {
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
    jsonToEventFactory.emplace("t", ToFlowEvent);
    jsonToEventFactory.emplace("C", ToCounterEvent);
}

void EventUtil::UnRegister()
{
    jsonToEventFactory.clear();
}

std::string EventUtil::Type(const json_t &json)
{
    if (json.HasMember("ph") && json["ph"].IsString()) {
        return json["ph"].GetString();
    }
    return "";
}

std::unique_ptr<Event> EventUtil::FromJson(const json_t &json)
{
    const std::string type = Type(json);
    if (type.empty()) {
        ServerLog::Warn("Can't find the type. json:", JsonUtil::JsonDump(json));
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
    event->pid = JsonUtil::GetDumpString(json, "pid");
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
    event->pid = JsonUtil::GetDumpString(json, "pid");
    if (json.HasMember("args")) {
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
    event->pid = JsonUtil::GetDumpString(json, "pid");
    event->flowId = JsonUtil::GetDumpString(json, "id");
    event->name = JsonUtil::GetString(json, "name");
    event->cat = JsonUtil::GetOptionalString(json, "cat");
    return event;
}

std::unique_ptr<Event> EventUtil::ToCounterEvent(const json_t &json)
{
    std::unique_ptr<Counter> event = std::make_unique<Counter>();
    event->type = Type(json);
    if (json.HasMember("id")) {
        event->name = JsonUtil::GetString(json, "name") + "[" + std::to_string(JsonUtil::GetInteger(json, "id")) + "]";
    } else {
        event->name = JsonUtil::GetString(json, "name");
    }
    event->pid = JsonUtil::GetDumpString(json, "pid");
    event->ts = JsonUtil::GetDouble(json, "ts");
    event->cat = JsonUtil::GetOptionalString(json, "cat");
    event->args = JsonUtil::GetDumpString(json, "args");
    return event;
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic