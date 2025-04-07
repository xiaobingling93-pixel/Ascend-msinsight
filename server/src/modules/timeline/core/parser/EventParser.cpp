/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "pch.h"
#include "EventUtil.h"
#include "TrackInfoManager.h"
#include "ParserStatusManager.h"
#include "SimulationSliceCacheManager.h"
#include "FileReader.h"
#include "EventParser.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
using json_t = rapidjson::Value;
EventParser::EventParser(const std::string &filePath, const std::string &fileId,
    std::shared_ptr<TextTraceDatabase> textDatabase)
    : filePath(filePath), fileId(fileId), database(std::move(textDatabase))
{
    ServerLog::Info("Init event parser. fileId:", fileId);
    InitEventHandle();
    fileReader = std::make_unique<FileReader>();
}

void EventParser::InitEventHandle()
{
    eventHandleMap.emplace("M", std::bind(&EventParser::MetaDataHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("X", std::bind(&EventParser::CompleteEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("I", std::bind(&EventParser::CompleteEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("C", std::bind(&EventParser::CounterEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("s", std::bind(&EventParser::FlowEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("t", std::bind(&EventParser::FlowEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("f", std::bind(&EventParser::FlowEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("SX", std::bind(&EventParser::SimulationEventHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("SB", std::bind(&EventParser::SimulationBeginEventHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("SE", std::bind(&EventParser::SimulationEndEventHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("Ss", std::bind(&EventParser::SimulationFlowEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("St", std::bind(&EventParser::SimulationFlowEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("SM", std::bind(&EventParser::MetaDataHandle, this, std::placeholders::_1));
}

bool EventParser::Parse(int64_t startPosition, int64_t endPosition)
{
    database->InitStmt();
    std::string buffer = fileReader->ReadJsonArray(filePath, startPosition, endPosition);
    if (buffer.empty()) {
        error = "Failed to read file when parse. file path is: " + filePath;
        ServerLog::Error("Event parser failed to read buffer. fileId:", fileId);
        return false;
    }
    auto data = JsonUtil::TryParse<kParseNumbersAsStringsFlag>(buffer, error);
    if (!data.has_value()) {
        error = "File is not valid json. " + error;
        ServerLog::Error("Event Parser. fileId:", fileId, ". ", error);
        return false;
    }
    if (!data.value().IsArray()) {
        error = "json is not an array.";
        ServerLog::Error("Event Parser. json is not an array. fileId:", fileId);
        return false;
    }
    for (auto &event : data.value().GetArray()) {
        if (ParserStatusManager::Instance().GetParserStatus(fileId) != ParserStatus::RUNNING) {
            return false;
        }
        EventHandle(event);
    }
    ProcessLastFlagSlice();
    database->CommitData();
    ServerLog::Info("Event Parser. fileId:", fileId, " Parse ", startPosition, " to ", endPosition,
        ". Count:", parseCount, ", ignore Count:", ignoreCount);
    return true;
}

void EventParser::ProcessLastFlagSlice()
{
    if (!waitFlagSliceMap.empty() || !setFlagSliceMap.empty()) {
        std::vector<Slice> sliceVec =
            SimulationSliceCacheManager::Instance().GetCompeteSlice(setFlagSliceMap, waitFlagSliceMap, fileId);
        for (const auto &item : sliceVec) {
            database->InsertSlice(item);
        }
    }
}


void EventParser::SetSimulationStatus(const bool &isSimulation)
{
    m_isSimulation = isSimulation;
}

void EventParser::EventHandle(const json_t &json)
{
    std::string type = EventUtil::Type(json);
    if (m_isSimulation) {
        type = "S" + type;
    }
    if (type.empty()) {
        return;
    }
    if (eventHandleMap.count(type) > 0) {
        parseCount++;
        eventHandleMap.at(type)(EventUtil::Instance().FromJson(json, type));
    } else {
        ignoreCount++;
    }
}

void EventParser::MetaDataHandle(std::unique_ptr<Trace::Event> eventPtr)
{
    if (eventPtr == nullptr) {
        return;
    }
    auto &event = dynamic_cast<Trace::MetaData &>(*eventPtr);
    if (event.name == "process_name") {
        database->UpdateProcessName(event);
    } else if (event.name == "thread_name") {
        event.trackId = GetTrackId(event.pid, event.tid);
        database->UpdateThreadName(event);
    } else if (event.name == "process_labels") {
        database->UpdateProcessLabel(event);
    } else if (event.name == "process_sort_index") {
        database->UpdateProcessSortIndex(event);
    } else if (event.name == "thread_sort_index") {
        event.trackId = GetTrackId(event.pid, event.tid);
        database->UpdateThreadSortIndex(event);
    } else {
        ServerLog::Error("Event Parser. Failed to get meta data type. name: %", event.name);
    }
}

void EventParser::CompleteEventsHandle(std::unique_ptr<Trace::Event> eventPtr)
{
    if (eventPtr == nullptr) {
        return;
    }
    auto &event = dynamic_cast<Trace::Slice &>(*eventPtr);
    event.trackId = GetTrackId(event.pid, event.tid);
    event.end = event.ts + event.dur;
    ProcessEvent processEvent;
    processEvent.pid = event.pid;
    processEvent.processName = event.pid;
    ThreadEvent threadEvent;
    threadEvent.trackId = event.trackId;
    threadEvent.tid = event.tid;
    threadEvent.pid = event.pid;
    threadEvent.threadName = event.tid;
    database->AddSimulationProcessCache(processEvent);
    database->AddSimulationThreadCache(threadEvent);
    database->InsertSlice(event);
}

void EventParser::SimulationBeginEventHandle(std::unique_ptr<Trace::Event> eventPtr)
{
    if (eventPtr == nullptr) {
        return;
    }
    auto &event = dynamic_cast<Trace::Slice &>(*eventPtr);
    if (event.name == "SET_FLAG" || event.name == "set_event") {
        if (setFlagSliceMap.find(event.flagId) == setFlagSliceMap.end()) {
            setFlagSliceMap[event.flagId] = event;
            return;
        } else {
            event.dur = setFlagSliceMap[event.flagId].ts > event.ts ? setFlagSliceMap[event.flagId].ts - event.ts : 0;
            SimulationEventHandle(std::move(eventPtr));
            setFlagSliceMap.erase(event.flagId);
            return;
        }
    }

    if (event.name == "WAIT_FLAG" || event.name == "wait_event") {
        if (waitFlagSliceMap.find(event.flagId) == waitFlagSliceMap.end()) {
            waitFlagSliceMap[event.flagId] = event;
            return;
        } else {
            event.dur = waitFlagSliceMap[event.flagId].ts > event.ts ?
                waitFlagSliceMap[event.flagId].ts - event.ts : 0;
            SimulationEventHandle(std::move(eventPtr));
            waitFlagSliceMap.erase(event.flagId);
            return;
        }
    }
}

void EventParser::SimulationEndEventHandle(std::unique_ptr<Trace::Event> eventPtr)
{
    if (eventPtr == nullptr) {
        return;
    }
    auto &event = dynamic_cast<Trace::Slice &>(*eventPtr);

    if (event.name == "SET_FLAG" || event.name == "set_event") {
        if (setFlagSliceMap.find(event.flagId) == setFlagSliceMap.end()) {
            setFlagSliceMap[event.flagId] = event;
            return;
        } else {
            Trace::Slice beginSlice = setFlagSliceMap[event.flagId];
            beginSlice.dur = event.ts > beginSlice.ts ? event.ts - beginSlice.ts : 0;
            SimulationEventHandle(std::make_unique<Trace::Slice>(beginSlice));
            setFlagSliceMap.erase(event.flagId);
            return;
        }
    }

    if (event.name == "WAIT_FLAG" || event.name == "wait_event") {
        if (waitFlagSliceMap.find(event.flagId) == waitFlagSliceMap.end()) {
            waitFlagSliceMap[event.flagId] = event;
            return;
        } else {
            Trace::Slice beginSlice = waitFlagSliceMap[event.flagId];
            beginSlice.dur = event.ts > beginSlice.ts ? event.ts - beginSlice.ts : 0;
            SimulationEventHandle(std::make_unique<Trace::Slice>(beginSlice));
            waitFlagSliceMap.erase(event.flagId);
            return;
        }
    }
}

void EventParser::SimulationEventHandle(std::unique_ptr<Trace::Event> eventPtr)
{
    if (eventPtr == nullptr) {
        return;
    }
    auto &event = dynamic_cast<Trace::Slice &>(*eventPtr);
    if (event.processName.empty() || event.threadName.empty()) {
        ServerLog::Error("processName or threadName is empty");
        return;
    }
    event.pid = event.processName;
    event.tid = event.threadName;
    event.trackId = GetTrackId(event.pid, event.tid);
    event.end = event.ts + event.dur;
    ThreadEvent threadEvent;
    threadEvent.trackId = event.trackId;
    threadEvent.tid = event.tid;
    threadEvent.pid = event.pid;
    threadEvent.threadName = event.threadName;
    threadEvent.SetThreadSortIndex();
    ProcessEvent processEvent;
    processEvent.pid = event.pid;
    processEvent.processName = event.processName;
    database->InsertSlice(event);
    database->AddSimulationProcessCache(processEvent);
    database->AddSimulationThreadCache(threadEvent);
}

void EventParser::FlowEventsHandle(std::unique_ptr<Trace::Event> eventPtr)
{
    if (eventPtr == nullptr) {
        return;
    }
    auto &event = dynamic_cast<Trace::Flow &>(*eventPtr);
    event.trackId = GetTrackId(event.pid, event.tid);
    ThreadEvent threadEvent;
    threadEvent.trackId = event.trackId;
    threadEvent.tid = event.tid;
    threadEvent.pid = event.pid;
    threadEvent.threadName = event.tid;
    ProcessEvent processEvent;
    processEvent.pid = event.pid;
    processEvent.processName = event.pid;
    database->AddSimulationProcessCache(processEvent);
    database->AddSimulationThreadCache(threadEvent);
    database->InsertFlow(event);
}

void EventParser::SimulationFlowEventsHandle(std::unique_ptr<Trace::Event> eventPtr)
{
    if (eventPtr == nullptr) {
        return;
    }
    auto &event = dynamic_cast<Trace::Flow &>(*eventPtr);
    std::string processName = event.pid;
    std::string threadName = event.tid;
    event.trackId = GetTrackId(event.pid, event.tid);
    ThreadEvent threadEvent;
    threadEvent.trackId = event.trackId;
    threadEvent.tid = event.tid;
    threadEvent.pid = event.pid;
    threadEvent.threadName = threadName;
    threadEvent.SetThreadSortIndex();
    ProcessEvent processEvent;
    processEvent.pid = event.pid;
    processEvent.processName = processName;
    database->AddSimulationProcessCache(processEvent);
    database->AddSimulationThreadCache(threadEvent);
    database->InsertFlow(event);
}

void EventParser::CounterEventsHandle(std::unique_ptr<Trace::Event> eventPtr)
{
    if (eventPtr == nullptr) {
        return;
    }
    auto &event = dynamic_cast<Trace::Counter &>(*eventPtr);
    if (event.ts == 0) {
        return;
    }
    event.trackId = GetTrackId(event.pid, event.tid);
    ThreadEvent threadEvent;
    threadEvent.trackId = event.trackId;
    threadEvent.tid = event.tid;
    threadEvent.pid = event.pid;
    threadEvent.threadName = event.tid;
    ProcessEvent processEvent;
    processEvent.pid = event.pid;
    processEvent.processName = event.pid;
    database->AddSimulationProcessCache(processEvent);
    database->AddSimulationThreadCache(threadEvent);
    database->InsertCounter(event);
}

uint64_t EventParser::GetTrackId(const std::string &pid, const std::string &tid)
{
    std::string str = pid + tid;
    if (trackIdMap.count(str) > 0) {
        return trackIdMap.at(str);
    }
    uint64_t id = TrackInfoManager::Instance().GetTrackId(fileId, pid, tid);
    trackIdMap.emplace(str, id);
    return id;
}

std::string EventParser::GetError()
{
    return error;
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
