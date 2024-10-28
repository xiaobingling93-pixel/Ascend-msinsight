/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATORGROUPQUERY_H
#define PROFILER_SERVER_COMMUNICATORGROUPQUERY_H

#include <string>
#include "ProtocolMessage.h"
#include "ProtocolDefs.h"
#include "CommunicationRequestHandler.h"
#include "CommunicationProtocolResponse.h"

namespace Dic {
namespace Module {
namespace Communication {
class CommunicatorGroupQueryHandler : public CommunicationRequestHandler {
public:
    CommunicatorGroupQueryHandler()
    {
        command = Protocol::REQ_RES_COMMUNICATION_COMMUNICATOR;
    };
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Communication
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_COMMUNICATORGROUPQUERY_H
