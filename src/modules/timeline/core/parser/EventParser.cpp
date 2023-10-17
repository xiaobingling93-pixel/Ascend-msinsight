/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <fstream>
#include "ServerLog.h"
#include "JsonUtil.h"
#include "EventUtil.h"
#include "TraceFileParser.h"
#include "EventParser.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
using json_t = rapidjson::Value;
EventParser::EventParser(const std::string &filePath, const std::string &dbPath,
                         const std::string &fileId) : filePath(filePath), dbPath(dbPath), fileId(fileId)
{
    ServerLog::Info("Init event parser. fileId:", fileId);
    InitEventHandle();
}

void EventParser::InitEventHandle()
{
    eventHandleMap.emplace("M", std::bind(&EventParser::MetaDataHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("X", std::bind(&EventParser::CompleteEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("s", std::bind(&EventParser::FlowEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("t", std::bind(&EventParser::FlowEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("f", std::bind(&EventParser::FlowEventsHandle, this, std::placeholders::_1));
}

void EventParser::Parse(int64_t startPosition, int64_t endPosition)
{
    database = std::make_unique<TraceDatabase>();
    database->OpenDb(dbPath, false);
    database->InitStmt();
    database->SetConfig();
    std::string buffer = ReadBuffer(startPosition, endPosition);
    if (buffer.empty()) {
        ServerLog::Error("EventParser. Failed to read buffer.");
        return;
    }
    rapidjson::Document d;
    try {
        d.Parse(buffer.c_str(), buffer.length());
    } catch (std::exception &e) {
        ServerLog::Error("EventParser. Failed to parse json. ", e.what());
        return;
    }
    for (auto &event : d.GetArray()) {
        EventHandle(event);
    }
    database->CommitData();
    database->ReleaseStmt();
    database->CloseDb();
    ServerLog::Info("EventParser. Parse ", startPosition, " to ", endPosition,
                    ". Count:", parseCount, ", ignore Count:", ignoreCount);
}

std::string EventParser::ReadBuffer(int64_t startPosition, int64_t endPosition)
{
#ifdef _WIN32
    std::string path(filePath);
    if (StringUtil::IsUtf8String(filePath)) {
        path = StringUtil::Utf8ToGbk(filePath.c_str());
    }
    std::ifstream file(path, std::ios::in);
#else
    std::ifstream file(filePath, std::ios::in);
#endif
    if (!file.is_open()) {
        ServerLog::Error("EventParser. Failed to open file.");
        return "";
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
    database->InsertSlice(event);
}

void EventParser::FlowEventsHandle(std::unique_ptr<Trace::Event> eventPtr)
{
    if (eventPtr == nullptr) {
        return;
    }
    auto &event = dynamic_cast<Trace::Flow &>(*eventPtr);
    event.trackId = GetTrackId(event.pid, event.tid);
    database->InsertFlow(event);
}

int64_t EventParser::GetTrackId(const std::string &pid, int64_t tid)
{
    std::string str = pid + std::to_string(tid);
    if (trackIdMap.count(str) > 0) {
        return trackIdMap.at(str);
    }
    int64_t id = TraceFileParser::Instance().GetTrackId(fileId, pid, tid);
    trackIdMap.emplace(str, id);
    return id;
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
