/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATION_ADVISOR_HANDLER_H
#define PROFILER_SERVER_COMMUNICATION_ADVISOR_HANDLER_H

#include "ProtocolDefs.h"
#include "CommunicationRequestHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
class CommunicationAdvisorHandler : public CommunicationRequestHandler {
public:
    CommunicationAdvisorHandler()
    {
        command = Protocol::REQ_RES_COMMUNICATION_ADVISOR;
    };
    ~CommunicationAdvisorHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Communication
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_COMMUNICATION_ADVISOR_HANDLER_H
