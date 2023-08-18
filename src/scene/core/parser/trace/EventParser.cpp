/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <fstream>
#include "ServerLog.h"
#include "JsonUtil.h"
#include "EventParser.h"
#include "DataBaseManager.h"
#include "TraceFileParser.h"

namespace Dic {
namespace Scene {
namespace Core {
using namespace Dic::Server;
EventParser::EventParser(const std::string &filePath, const std::string &dbPath, const std::string &fileId) : filePath(filePath), dbPath(dbPath), fileId(fileId)
{
    ServerLog::Info("Init event parser. fileId:", fileId);
    database = DataBaseManager::Instance().GetTraceDatabase(fileId);
    InitEventHandle();
}

void EventParser::InitEventHandle()
{
    eventHandleMap.emplace("M", std::bind(&EventParser::MetaDataHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("X", std::bind(&EventParser::CompleteEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("s", std::bind(&EventParser::FlowEventsHandle, this, std::placeholders::_1));
    eventHandleMap.emplace("f", std::bind(&EventParser::FlowEventsHandle, this, std::placeholders::_1));
}

void EventParser::Parse(uint64_t startPosition, uint64_t endPosition)
{
    uint64_t len = endPosition - startPosition + 1 + 2; // + '[' ']'
    auto buffer = std::make_unique<char[]>(endPosition - startPosition + 1);
    if (!ReadBuffer(buffer.get() + 1, startPosition, endPosition)) {
        ServerLog::Error("EventParser. Failed to read buffer.");
        return;
    }
    buffer[0] ='[';
    buffer[len - 1] = ']';
    std::string error;
    auto json = JsonUtil::TryParse(buffer.get(), len, error);
    if (!(json.has_value() && json.value().is_array())) {
        ServerLog::Error("EventParser. Failed to parse json string. error:", error);
        return;
    }
    for (auto &event : json.value()) {
        EventHandle(event);
    }
    database->ReStartTransaction();
    ServerLog::Info("EventParser. Parse ", startPosition, " to ", endPosition,
                    ". Count:", parseCount, ", ignore Count:", ignoreCount);
}

bool EventParser::ReadBuffer(char *buffer, uint64_t startPosition, uint64_t endPosition)
{
    std::ifstream file(filePath, std::ios::in);
    if (!file.is_open()) {
        ServerLog::Error("EventParser. Failed to open file.");
        return false;
    }
    file.seekg(startPosition, std::ios::beg);
    if (!file.read(buffer, endPosition - startPosition + 1)) {
        file.close();
        ServerLog::Error("EventParser. Failed to read file. start:", startPosition, ", end:", endPosition);
        return false;
    }
    file.close();
    return true;
}

void EventParser::EventHandle(json_t &json)
{
    std::string type;
    try {
        type = json["ph"];
    } catch (std::exception &e) {
        ServerLog::Error("EventHandle json:", json.dump(), ". error:", e.what());
        return;
    }
    if (eventHandleMap.count(type) > 0) {
        parseCount++;
        eventHandleMap.at(type)(json);
    } else {
        ignoreCount++;
    }
}

void EventParser::MetaDataHandle(json_t &json)
{
    std::string name = JsonUtil::GetString(json, "name");
    if (name.empty()) {
        ServerLog::Error("EventParser. Failed to get event name. json:", json.dump());
        return;
    }
    if (name == "process_name") {
        database->UpdateProcessName(json);
    } else if (name == "thread_name") {
        AddTrackId(json);
        database->UpdateThreadName(json);
    } else if (name == "process_labels") {
        database->UpdateProcessLabel(json);
    } else if (name == "process_sort_index") {
        database->UpdateProcessSortIndex(json);
    } else if (name == "thread_sort_index") {
        AddTrackId(json);
        database->UpdateThreadSortIndex(json);
    } else {
        ServerLog::Error("EventParser. Failed to get meta data type. json:", json.dump());
    }
}

void EventParser::CompleteEventsHandle(json_t &json)
{
    AddTrackId(json);
    database->InsertSlice(json);
}

void EventParser::FlowEventsHandle(json_t &json)
{
    AddTrackId(json);
    database->InsertFlow(json);
}

void EventParser::AddTrackId(json_t &json)
{
    try {
        json["track_id"] = GetTrackId(json["pid"], json["tid"]);
    } catch (std::exception &e) {
        ServerLog::Error("EventParser. Failed to parse flow event. error", e.what(), ". json:", json.dump());
    }
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
} // end of namespace Core
} // end of namespace Scene
} // end of namespace Dic
