/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Event declaration
 */

#ifndef DIC_PROTOCOL_EVENT_H
#define DIC_PROTOCOL_EVENT_H

#include <vector>
#include <memory>
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
struct UnitMetaData {
    std::string cardId;
};

struct UnitTrackMetaData {
    std::string cardId;
    std::string processId;
    std::string processName; // type = process
    std::string label; // type = process
    std::string threadId; // type = thread
    std::string threadName; // type = thread, counter
    int maxDepth = 0; // type = thread
    std::vector<std::string> dataType; // type = counter
};

struct UnitTrack {
    std::string type;
    UnitTrackMetaData metaData;
    std::vector<std::unique_ptr<UnitTrack>> children;
};

struct Unit {
    std::string type;
    UnitMetaData metadata;
    std::vector<std::unique_ptr<UnitTrack>> children;
};

struct ParseSuccessEventBody {
    Unit unit;
    bool startTimeUpdated = false;
    uint64_t maxTimeStamp = 0;
};

struct ParseSuccessEvent : public Event {
    ParseSuccessEvent() : Event(EVENT_PARSE_SUCCESS) {}
    ParseSuccessEventBody body;
};

struct ParseFailEventBody {
    std::string rankId;
    std::string error;
};

struct ParseFailEvent : public Event {
    ParseFailEvent() : Event(EVENT_PARSE_FAIL) {}
    ParseFailEventBody body;
};

struct MemorySuccess {
    std::string rankId;
    bool parseSuccess = false;
    bool hasFile = false;
};

struct ParseClusterCompletedEventBody {
    std::string parseResult;
};

struct ParseClusterCompletedEvent : public Event {
    ParseClusterCompletedEvent() : Event(EVENT_PARSE_CLUSTER_COMPLETED) {}
    ParseClusterCompletedEventBody body;
};

struct ParseMemoryCompletedEvent : public Event {
    ParseMemoryCompletedEvent() : Event(EVENT_PARSE_MEMORY_COMPLETED) {}
    bool isCluster = false;
    std::vector<MemorySuccess> memoryResult;
};

} // end of namespace Protocol
} // end if namespace Dic

#endif // DIC_PROTOCOL_EVENT_H
