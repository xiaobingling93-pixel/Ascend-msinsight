/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "ProtocolDefs.h"
#include "ProtocolEnumUtil.h"
#include "EventUtil.h"
#include "ProtocolUtil.h"
#include "ProtocolEvent.h"
#include "JsonUtil.h"
#include "EventManager.h"

namespace Dic {
namespace Protocol {
using namespace Dic;
using namespace Dic::Protocol;
using namespace Dic::Server;
EventManager::EventManager()
{
    Register();
}

EventManager::~EventManager()
{
    UnRegister();
}

void EventManager::Register()
{
    std::lock_guard<std::mutex> lock(mutex);
    RegisterJsonToEventFuncs();
    RegisterEventToJsonFuncs();
}

void EventManager::RegisterJsonToEventFuncs()
{

}

void EventManager::RegisterEventToJsonFuncs()
{
    eventToJsonFactory.emplace(EVENT_DEVICE_CHANGED, ToDeviceChangedEventJson);
    eventToJsonFactory.emplace(EVENT_PARSE_SUCCESS, ToParseSuccessEventJson);
}

void EventManager::UnRegister()
{
    std::lock_guard<std::mutex> lock(mutex);
    eventToJsonFactory.clear();
}

std::optional<EventManager::JsonToEventFunc> EventManager::GetJsonToEventFunc(const std::string &event)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (jsonToEventFactory.count(event) == 0) {
        return std::nullopt;
    }
    return jsonToEventFactory[event];
}

std::optional<EventManager::EventToJsonFunc> EventManager::GetEventToJsonFunc(const std::string &event)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (eventToJsonFactory.count(event) == 0) {
        return std::nullopt;
    }
    return eventToJsonFactory[event];
}

bool EventManager::IsEvent(const json_t &jsonEvent) const
{
    return (jsonEvent.contains("type")) && (jsonEvent["type"] == EVENT_NAME);
}

const std::string EventManager::GetEventName(const json_t &jsonEvent) const
{
    if (jsonEvent.contains("event") && jsonEvent["event"].is_string()) {
        return jsonEvent["event"].get<std::string>();
    }
    return "";
}

const std::unique_ptr<Event> EventManager::FromJson(const json_t &eventJson, std::string &error)
{
    if (!IsEvent(eventJson)) {
        error = "json type is not event.";
        return nullptr;
    }
    const std::string command = GetEventName(eventJson);
    std::optional<EventManager::JsonToEventFunc> func = GetJsonToEventFunc(command);
    if (!func.has_value()) {
        return nullptr;
    }
    return func.value()(eventJson, error);
}

const std::unique_ptr<Event> EventManager::FromJson(const std::string &eventStr, std::string &error)
{
    json_t eventJson;
    try {
        eventJson = json_t::parse(eventStr);
    } catch (json_t::parse_error &) {
        return nullptr;
    }
    return FromJson(eventJson, error);
}

std::optional<json_t> EventManager::ToJson(const Event &event, std::string &error)
{
    std::string eventString = event.event;
    std::optional<EventManager::EventToJsonFunc> func = GetEventToJsonFunc(eventString);
    if (!func.has_value()) {
        error = "Failed to find event target function, token = " + event.token + ", event = " + eventString;
        return std::nullopt;
    }
    try {
        return func.value()(event);
    } catch (std::exception &e) {
        error = "Failed to convert event to json, event = " + eventString;
        return std::nullopt;
    }
}
#pragma region << Event To Json>>

std::optional<json_t> EventManager::ToDeviceChangedEventJson(const Event &event)
{
    return ToEventJson<DeviceChangedEvent>(dynamic_cast<const DeviceChangedEvent &>(event));
}

std::optional<json_t> EventManager::ToParseSuccessEventJson(const Event &event)
{
    return ToEventJson<ParseSuccessEvent>(dynamic_cast<const ParseSuccessEvent &>(event));
}

#pragma endregion

#pragma region << Json To Event>>
// no need
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic