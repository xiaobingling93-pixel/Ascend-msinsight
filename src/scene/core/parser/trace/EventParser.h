/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SCENE_CORE_EVENT_PARSER_H
#define DATA_INSIGHT_CORE_SCENE_CORE_EVENT_PARSER_H

#include <string>
#include <map>
#include <functional>
#include "GlobalDefs.h"
#include "TraceDatabase.h"

namespace Dic {
namespace Scene {
namespace Core {
class EventParser {
public:
    EventParser(const std::string &filePath, const std::string &dbPath, const std::string &fileId);
    ~EventParser() = default;
    void Parse(uint64_t startPosition, uint64_t endPosition);

private:
    std::string filePath;
    std::string dbPath;
    std::string fileId;
    int parseCount = 0;
    int ignoreCount = 0;
    std::unique_ptr<TraceDatabase> database;
    std::map<std::string, std::function<void(std::unique_ptr<Trace::Event>)>> eventHandleMap;

    bool ReadBuffer(char *buffer, uint64_t startPosition, uint64_t endPosition);
    void EventHandle(json_t &json);
    void InitEventHandle();
    void MetaDataHandle(std::unique_ptr<Trace::Event> eventPtr);
    void CompleteEventsHandle(std::unique_ptr<Trace::Event> eventPtr);
    void FlowEventsHandle(std::unique_ptr<Trace::Event> eventPtr);
    std::map<std::string, int64_t> trackIdMap;
    int64_t GetTrackId(const std::string &pid, int64_t tid);
};
} // end of namespace Core
} // end of namespace Scene
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SCENE_CORE_EVENT_PARSER_H
