/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol event manager declaration
 */

#ifndef DIC_PROTOCOL_EVENT_MANAGER_H
#define DIC_PROTOCOL_EVENT_MANAGER_H

#include <string>
#include <map>
#include <functional>
#include <mutex>
#include <optional>
#include "GlobalDefs.h"
#include "ProtocolBase.h"

namespace Dic {
namespace Protocol {
class EventManager {
public:
    static EventManager &Instance()
    {
        static EventManager instance;
        return instance;
    }
    bool IsEvent(const json_t &jsonEvent) const;
    const std::string GetEventName(const json_t &jsonEvent) const;
    const std::unique_ptr<Event> FromJson(const json_t &eventJson, std::string &error);
    const std::unique_ptr<Event> FromJson(const std::string &eventStr, std::string &error);
    std::optional<json_t> ToJson(const Event &event, std::string &error);

    using JsonToEventFunc = std::function<std::unique_ptr<Event>(const json_t &, std::string &error)>;
    std::optional<JsonToEventFunc> GetJsonToEventFunc(const std::string &event);
    using EventToJsonFunc = std::function<std::optional<json_t>(const Event &)>;
    std::optional<EventToJsonFunc> GetEventToJsonFunc(const std::string &command);

private:
    EventManager();
    ~EventManager();

    void Register();
    void RegisterJsonToEventFuncs();
    void RegisterEventToJsonFuncs();
    void UnRegister();

    static std::optional<json_t> ToParseSuccessEventJson(const Event &event);
    static std::optional<json_t> ToDeviceChangedEventJson(const Event &event);

    std::mutex mutex;
    std::map<std::string, JsonToEventFunc> jsonToEventFactory;
    std::map<std::string, EventToJsonFunc> eventToJsonFactory;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_PROTOCOL_EVENT_MANAGER_H
