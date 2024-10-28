/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_RANKSHANDLER_H
#define PROFILER_SERVER_RANKSHANDLER_H

#include <set>
#include <regex>
#include "ProtocolMessage.h"
#include "ProtocolDefs.h"
#include "CommunicationRequestHandler.h"

namespace Dic {
    namespace Module {
        namespace Communication {
class RanksHandler : public CommunicationRequestHandler {
public:
    RanksHandler()
    {
        command = Protocol::REQ_RES_COMMUNICATION_RANKS;
    }
    ~RanksHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
        } // end of namespace Communication
    } // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_RANKSHANDLER_H
