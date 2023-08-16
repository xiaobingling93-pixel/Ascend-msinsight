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
    EventParser(const std::string &filePath, const std::string &dbPath);
    ~EventParser() = default;
    void Parse(uint64_t startPosition, uint64_t endPosition);

private:
    std::string filePath;
    std::string dbPath;
    int parseCount = 0;
    int ignoreCount = 0;
    TraceDatabase database;
    std::map<std::string, std::function<void(json_t &json)>> eventHandleMap;

    bool ReadBuffer(char *buffer, uint64_t startPosition, uint64_t endPosition);
    void EventHandle(json_t &json);
    void InitEventHandle();
    void MetaDataHandle(json_t &json);
    void CompleteEventsHandle(json_t &json);
    void FlowEventsHandle(json_t &json);
    int id = 1;
    std::map<std::string, int> trackeMap;
    int64_t GetTrackId(const std::string &pid, int64_t tid);
    void AddTrackId(json_t &json);
};
} // end of namespace Core
} // end of namespace Scene
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SCENE_CORE_EVENT_PARSER_H
