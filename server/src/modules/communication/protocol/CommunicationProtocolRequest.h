/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef DIC_COMMUNICATION_PROTOCOL_COMMUNICATION_REQUEST_H
#define DIC_COMMUNICATION_PROTOCOL_COMMUNICATION_REQUEST_H

#include <string>
#include <optional>
#include <vector>
#include "ProtocolMessage.h"
#include "ProtocolDefs.h"

namespace Dic {
namespace Protocol {
struct OperatorDetailsParam {
    std::string iterationId;
    std::string rankId;
    std::string orderBy;
    std::string order;
    std::string stage;
    int pageSize;
    int currentPage;
};

struct OperatorDetailsRequest : public Request {
    OperatorDetailsRequest() : Request(REQ_RES_COMMUNICATION_OPERATOR_DETAILS) {};
    OperatorDetailsParam params;
};

struct BandwidthDataParam {
    std::string dbIndex;
    std::string iterationId;
    std::string rankId;
    std::string operatorName;
    std::string stage;
};

struct BandwidthDataRequest : public Request {
    BandwidthDataRequest() : Request(REQ_RES_COMMUNICATION_BANDWIDTH) {};
    BandwidthDataParam params;
};

struct DistributionDataParam {
    std::string dbIndex;
    std::string iterationId;
    std::string rankId;
    std::string operatorName;
    std::string transportType;
    std::string stage;
};

struct DistributionDataRequest : public Request {
    DistributionDataRequest() : Request(REQ_RES_COMMUNICATION_DISTRIBUTION) {};
    DistributionDataParam params;
};

struct CommunicatorGroupParam {
    std::string filePath;
};

struct CommunicatorGroupRequest : public Request {
    CommunicatorGroupRequest() : Request(REQ_RES_COMMUNICATOR_PARSE) {};
    CommunicatorGroupParam params;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_COMMUNICATION_PROTOCOL_COMMUNICATION_REQUEST_H