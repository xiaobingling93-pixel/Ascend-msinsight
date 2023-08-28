/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Event declaration
 */

#ifndef DIC_PROTOCOL_EVENT_H
#define DIC_PROTOCOL_EVENT_H

#include "ProtocolDefs.h"
#include "ProtocolBase.h"
#include "ProtocolEntity.h"

namespace Dic {
namespace Protocol {
// global
struct InitializedEventBody {};

struct InitializedEvent : public Event {
    InitializedEvent() : Event(EVENT_INITIALIZED) {}
    InitializedEventBody body;
};

// timeline
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
