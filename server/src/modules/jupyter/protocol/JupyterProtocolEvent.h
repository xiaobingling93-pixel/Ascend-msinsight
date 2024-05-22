/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_JUPYTERPROTOCOLEVENT_H
#define PROFILER_SERVER_JUPYTERPROTOCOLEVENT_H

#include "string"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
struct ParseJupyterCompletedEventBody {
    std::string parseResult;
    std::string url;
};
struct ParseJupyterCompletedEvent : public Event {
    ParseJupyterCompletedEvent() : Event(EVENT_PARSE_JUPYTER_COMPLETED) {}
    ParseJupyterCompletedEventBody body;
};
}
}

#endif // PROFILER_SERVER_JUPYTERPROTOCOLEVENT_H
