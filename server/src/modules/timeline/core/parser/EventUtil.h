/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_EVENT_UTIL_H
#define PROFILER_SERVER_EVENT_UTIL_H

#include <string>
#include <map>
#include <optional>
#include <memory>
#include <functional>
#include "GlobalDefs.h"
#include "EventDef.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Trace;
class EventUtil {
public:
    static EventUtil &Instance()
    {
        static EventUtil instance;
        return instance;
    }
    using json_t = rapidjson::Value;
    static std::string Type(const json_t &json) ;
    Trace::Event* FromJson(const json_t &json, const std::string &type);

private:
    EventUtil();
    ~EventUtil();

    void Register();
    void UnRegister();

    using JsonToEventFunc = std::function<Trace::Event*(const json_t &)>;
    std::map<std::string, JsonToEventFunc> jsonToEventFactory;
    std::optional<JsonToEventFunc> GetJsonToEventFunc(const std::string &type);
    static Trace::Event* ToSliceEvent(const json_t &json);
    static Trace::Event* ToSimulationSliceEvent(const json_t &json);
    static Trace::Event* ToSimulationBeginSliceEvent(const json_t &json);
    static Trace::Event* ToSimulationEndSliceEvent(const json_t &json);
    static Trace::Event* ToMetaDataEvent(const json_t &json);
    static Trace::Event* ToFlowEvent(const json_t &json);
    static Trace::Event* ToCounterEvent(const json_t &json);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_EVENT_UTIL_H
