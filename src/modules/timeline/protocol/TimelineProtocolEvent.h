/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Event declaration
 */

#ifndef DIC_PROTOCOL_EVENT_H
#define DIC_PROTOCOL_EVENT_H

#include "ProtocolDefs.h"
#include "ProtocolBase.h"

namespace Dic {
namespace Protocol {
struct UnitMetaData {
    std::string cardId;
};

struct UnitTrackMeatData {
    std::string cardId;
    std::string processId;
    std::string processName;
    std::string label;
    int64_t threadId = 0;
    std::string threadName;
    int maxDepth = 0;
};

struct UnitTrack {
    std::string type;
    UnitTrackMeatData metaData;
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

} // end of namespace Protocol
} // end if namespace Dic

#endif // DIC_PROTOCOL_EVENT_H
