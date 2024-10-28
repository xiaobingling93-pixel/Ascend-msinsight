/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATIONOPERATORLISTSHANDLER_H
#define PROFILER_SERVER_COMMUNICATIONOPERATORLISTSHANDLER_H

#include "ProtocolMessage.h"
#include "ProtocolDefs.h"
#include "CommunicationRequestHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
class CommunicationOperatorListsHandler : public CommunicationRequestHandler {
public:
    CommunicationOperatorListsHandler()
    {
        command = Protocol::REQ_RES_COMMUNICATION_OPERATOR_LISTS;
    }
    ~CommunicationOperatorListsHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};

} // Dic
} // Module
} // Communication

#endif // PROFILER_SERVER_COMMUNICATIONOPERATORLISTSHANDLER_H
