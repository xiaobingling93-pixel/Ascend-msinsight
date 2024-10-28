/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATION_DISTRIBUTION_HANDLER_H
#define PROFILER_SERVER_COMMUNICATION_DISTRIBUTION_HANDLER_H

#include <set>
#include <regex>
#include "ProtocolMessage.h"
#include "ProtocolDefs.h"
#include "CommunicationRequestHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
class DistributionHandler : public CommunicationRequestHandler {
public:
    DistributionHandler()
    {
        command = Protocol::REQ_RES_COMMUNICATION_DISTRIBUTION;
    };
    ~DistributionHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
} // end of namespace Communication
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_COMMUNICATION_DISTRIBUTION_HANDLER_H
