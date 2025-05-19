/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_IEPROTOCOLEVENT_H
#define PROFILER_SERVER_IEPROTOCOLEVENT_H
#include <vector>
#include <memory>
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"
namespace Dic::Protocol {
struct ParseStatisticCompletedEvent : public Event {
    ParseStatisticCompletedEvent() : Event(EVENT_PARSE_IE_COMPLETED) {}
    std::vector<std::string> rankIds;
};
}

#endif // PROFILER_SERVER_IEPROTOCOLEVENT_H
