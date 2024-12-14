/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_GLOBALPROTOCOLEVENT_H
#define PROFILER_SERVER_GLOBALPROTOCOLEVENT_H

#include "ProtocolDefs.h"
#include "ProtocolUtil.h"

namespace Dic::Protocol {
struct ReadFileFailEventBody {
    std::string filePath;
    std::string error;
};

struct ReadFileFailEvent : public Event {
    ReadFileFailEvent() : Event(EVENT_FILES_READ_FAIL) {}
    ReadFileFailEventBody body;
};

void SendReadFileFailEvent(const std::string &filePath, const std::string &errMsg);
}

#endif // PROFILER_SERVER_GLOBALPROTOCOLEVENT_H
