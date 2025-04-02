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
    std::string cardAlias;
};

struct UnitTrackMetaData {
    std::string cardId;
    std::string processId;
    std::string processName; // type = process
    std::string label;       // type = process
    std::string threadId;    // type = thread
    std::string threadName;  // type = thread, counter
    std::string groupNameValue;   // type = thread, DB 在 PROCESS_TYPE::HCCL 时赋值；TEXT 看 threadName 形式符合赋值
    std::vector<std::string> rankList; // type = thread, 为HCCL中group甬道时赋值，内容为通信域内所有rankId信息
    std::string metaType;
    int maxDepth = 0;                  // type = thread
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
    bool isFullDb = false;
    uint64_t maxTimeStamp = 0;
    uint64_t offset = 0;
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
    bool isShowCluster = false;
    bool isAllPageParsed = false;
};

struct ParseClusterCompletedEvent : public Event {
    ParseClusterCompletedEvent() : Event(EVENT_PARSE_CLUSTER_COMPLETED) {}
    ParseClusterCompletedEventBody body;
};

struct CardOffset {
    std::string cardId;
    uint64_t offset = 0;
};

struct AllSuccessEventEventBody {
    bool isAllPageParsed = false;
    std::vector<CardOffset> cardOffsets;
    uint64_t minTime = 0;
};

struct AllSuccessEvent : public Event {
    AllSuccessEvent() : Event(EVENT_ALL_SUCCESS) {}
    AllSuccessEventEventBody body;
};

struct ParseClusterStep2CompletedEvent : public Event {
    ParseClusterStep2CompletedEvent() : Event(EVENT_PARSE_CLUSTER_STEP2_COMPLETED) {}
    ParseClusterCompletedEventBody body;
};

struct ParseMemoryCompletedEvent : public Event {
    ParseMemoryCompletedEvent() : Event(EVENT_PARSE_MEMORY_COMPLETED) {}
    bool isCluster = false;
    std::vector<MemorySuccess> memoryResult;
};

struct ModuleResetEvent : public Event {
    ModuleResetEvent() : Event(EVENT_MODULE_RESET) {}
    bool reset = false;
};

struct ParseProgressEventBody {
    std::string fileId;
    uint64_t parsedSize = 0;
    uint64_t totalSize = 0;
    int progress = 0;
};

struct ParseProgressEvent : public Event {
    ParseProgressEvent() : Event(EVENT_PARSE_PROGRESS) {}
    ParseProgressEventBody body;
};
} // end of namespace Protocol
} // end if namespace Dic

#endif // DIC_PROTOCOL_EVENT_H
