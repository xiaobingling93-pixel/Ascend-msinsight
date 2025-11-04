/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_MODULE_CORE_EVENT_PARSER_H
#define DATA_INSIGHT_CORE_MODULE_CORE_EVENT_PARSER_H

#include <string>
#include <map>
#include <functional>
#include "GlobalDefs.h"
#include "TextTraceDatabase.h"
#include "IFileReader.h"
#include "EventDef.h"

namespace Dic {
namespace Module {
namespace Timeline {
class EventParser {
public:
    EventParser(const std::string &filePath, const std::string &fileId,
        std::shared_ptr<TextTraceDatabase> textDatabase);
    ~EventParser() = default;
    bool Parse(int64_t startPosition, int64_t endPosition);
    std::string GetError();
    void SetSimulationStatus(const bool &isSimulation);
protected:
    std::unique_ptr<IFileReader> fileReader = nullptr;

private:
    std::string filePath;
    std::string fileId;
    std::string error;
    int parseCount = 0;
    int ignoreCount = 0;
    bool m_isSimulation = false;
    std::shared_ptr<TextTraceDatabase> database;
    std::map<std::string, std::function<void(Trace::Event*)>> eventHandleMap;

    void EventHandle(const rapidjson::Value &json);
    void InitEventHandle();
    void MetaDataHandle(Trace::Event* eventPtr);
    void CompleteEventsHandle(Trace::Event* eventPtr);
    void SimulationBeginEventHandle(Trace::Event* eventPtr);
    void SimulationEndEventHandle(Trace::Event* eventPtr);
    void SimulationEventHandle(Trace::Event* eventPtr);
    void FlowEventsHandle(Trace::Event* eventPtr);
    void SimulationFlowEventsHandle(Trace::Event* eventPtr);
    void CounterEventsHandle(Trace::Event* eventPtr);
    std::map<std::string, uint64_t> trackIdMap;
    std::map<std::string, Trace::Slice> setFlagSliceMap;
    std::map<std::string, Trace::Slice> waitFlagSliceMap;
    uint64_t GetTrackId(const std::string &pid, const std::string &tid);

    void ProcessLastFlagSlice();
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_MODULE_CORE_EVENT_PARSER_H
