/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_OPERATORPROTOCOLEVENT_H
#define PROFILER_SERVER_OPERATORPROTOCOLEVENT_H

#include <vector>
#include "ProtocolDefs.h"
#include "GlobalDefs.h"
#include "ProtocolMessage.h"

namespace Dic::Protocol {
    struct OperatorRank {
        std::string rankId;
        bool status;
        std::string error;
    };

    struct OperatorParseStatusEvent : public Event {
        OperatorParseStatusEvent() : Event(EVENT_PARSE_OPERATOR_STATUS) {}
        OperatorRank data;
        std::string fileId;
        std::vector<RankInfo> rankList;
    };

    struct OperatorParseClearEvent : public Event {
        OperatorParseClearEvent() : Event(EVENT_PARSE_OPERATOR_CLEAR) {}
    };
}

#endif // PROFILER_SERVER_OPERATORPROTOCOLEVENT_H
