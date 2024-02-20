/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <fstream>
#include "ServerLog.h"
#include "JsonUtil.h"
#include "EventUtil.h"
#include "DataBaseManager.h"
#include "TraceFileParser.h"
#include "ParserStatusManager.h"
#include "EventParser.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
using json_t = rapidjson::Value;
EventParser::EventParser(const std::string &filePath, const std::string &fileId) : filePath(filePath), fileId(fileId)
{
    ServerLog::Info("Init event parser. fileId:", fileId);
    InitEventHandle();
}

void EventParser::InitEventHandle()
{
    eventHandleMap.emplace("M", std::bind(&EventParser::MetaDataHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("X", std::bind(&EventParser::CompleteEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("C", std::bind(&EventParser::CounterEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("s", std::bind(&EventParser::FlowEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("t", std::bind(&EventParser::FlowEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("f", std::bind(&EventParser::FlowEventsHandle, this, std::placeholders::_1));
}

bool EventParser::Parse(int64_t startPosition, int64_t endPosition)
{
    std::shared_ptr<TraceDatabase> databasePtr = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (databasePtr == nullptr) {
        error = "Failed to get connection. fileId:" + fileId;
        ServerLog::Error(error);
        return false;
    }
    databasePtr->InitStmt();
    std::string buffer = ReadBuffer(startPosition, endPosition);
    if (buffer.empty()) {
        error = "Failed to read file.";
        ServerLog::Error("EventParser. Failed to read buffer. fileId:", fileId);
        return false;
    }
    auto data = JsonUtil::TryParse<kParseNumbersAsStringsFlag>(buffer, error);
    if (!data.has_value()) {
        error = "File is not valid json. " + error;
        ServerLog::Error("EventParser. fileId:", fileId, ". ", error);
        return false;
    }
    if (!data.value().IsArray()) {
        error = "json is not an array.";
        ServerLog::Error("EventParser. json is not an array. fileId:", fileId);
        return false;
    }
    database = databasePtr;
    for (auto &event : data.value().GetArray()) {
        if (ParserStatusManager::Instance().GetParserStatus(fileId) != ParserStatus::RUNNING) {
            return false;
        }
        EventHandle(event);
    }
    database->CommitData();
    database.reset(); // return connection pool
    ServerLog::Info("EventParser. fileId:", fileId, " Parse ", startPosition, " to ", endPosition,
                    ". Count:", parseCount, ", ignore Count:", ignoreCount);
    return true;
}

std::string EventParser::ReadBuffer(int64_t startPosition, int64_t endPosition)
{
#ifdef _WIN32
    std::string path(filePath);
    if (StringUtil::IsUtf8String(filePath)) {
        path = StringUtil::Utf8ToGbk(filePath.c_str());
    }
    std::ifstream file(path, std::ios::in | std::ios::binary);
#else
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
#endif
    if (!file.is_open()) {
        ServerLog::Error("EventParser. Failed to open file.");
        return "";
    }
    if (startPosition == 0 && endPosition == 0) {
        std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return str;
    }
    file.seekg(startPosition, std::ios::beg);
    int64_t suffixLen = 2; // [ ]
    int64_t len = endPosition - startPosition + 1 + suffixLen; // + [ ] + \0
    auto buffer = std::make_unique<char[]>(len);
    if (!file.read(buffer.get() + 1, len - suffixLen)) { // reserved '[' and ']'
        file.close();
        ServerLog::Error("EventParser. Failed to read file. start:", startPosition, ", end:", endPosition);
        return "";
    }
    file.close();
    buffer[0] = '[';
    buffer[len - 1] = ']';
    return {buffer.get(), static_cast<uint64_t>(len)};
}

void EventParser::EventHandle(const json_t &json)
{
    std::string type = EventUtil::Type(json);
    if (type.empty()) {
        ServerLog::Error("EventHandle. event type is empty. ", JsonUtil::JsonDump(json));
        return;
    }
    if (eventHandleMap.count(type) > 0) {
        parseCount++;
        eventHandleMap.at(type)(EventUtil::Instance().FromJson(json));
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
        ServerLog::Error("EventParser. Failed to get meta data type. name:", event.name);
    }
}

void EventParser::CompleteEventsHandle(std::unique_ptr<Trace::Event> eventPtr)
{
    if (eventPtr == nullptr) {
        return;
    }
    auto &event = dynamic_cast<Trace::Slice &>(*eventPtr);
    event.trackId = GetTrackId(event.pid, event.tid);
    std::tuple<int64_t, std::string, std::string> thread = {event.trackId, event.tid, event.pid};
    database->AddThreadCache(thread);
    database->InsertSlice(event);
}

void EventParser::FlowEventsHandle(std::unique_ptr<Trace::Event> eventPtr)
{
    if (eventPtr == nullptr) {
        return;
    }
    auto &event = dynamic_cast<Trace::Flow &>(*eventPtr);
    event.trackId = GetTrackId(event.pid, event.tid);
    std::tuple<int64_t, std::string, std::string> thread = {event.trackId, event.tid, event.pid};
    database->AddThreadCache(thread);
    database->InsertFlow(event);
}

void EventParser::CounterEventsHandle(std::unique_ptr<Trace::Event> eventPtr)
{
    if (eventPtr == nullptr) {
        return;
    }
    database->InsertCounter(dynamic_cast<Trace::Counter &>(*eventPtr));
}

int64_t EventParser::GetTrackId(const std::string &pid, const std::string &tid)
{
    std::string str = pid + tid;
    if (trackIdMap.count(str) > 0) {
        return trackIdMap.at(str);
    }
    int64_t id = TraceFileParser::Instance().GetTrackId(fileId, pid, tid);
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
