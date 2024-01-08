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

struct CommunicatorGroupRequest : public Request {
    CommunicatorGroupRequest() : Request(REQ_RES_COMMUNICATION_COMMUNICATOR) {};
};

struct RanksParams {
    std::string dbIndex;
    std::string iterationId;
};

struct RanksRequest  : public Request {
    RanksRequest() : Request(REQ_RES_COMMUNICATION_RANKS) {};
    RanksParams params;
};

struct IterationsParams {
};

struct IterationsRequest  : public Request {
    IterationsRequest() : Request(REQ_RES_COMMUNICATION_ITERATIONS) {};
    IterationsParams params;
};

struct OperatorNamesParams {
    std::string dbIndex;
    std::string iterationId;
    std::vector<std::string> rankList = {};
    std::string stage;
};

struct OperatorNamesRequest  : public Request {
    OperatorNamesRequest() : Request(REQ_RES_COMMUNICATION_OPERATORNAMES) {};
    OperatorNamesParams params;
};

struct DurationListParams {
    std::string dbIndex;
    std::string iterationId;
    std::vector<std::string> rankList = {};
    std::string operatorName;
    std::string stage;
};

struct DurationListRequest  : public Request {
    DurationListRequest() : Request(REQ_RES_COMMUNICATION_LIST) {};
    DurationListParams params;
};

struct MatrixGroupParam {
    std::string iterationId;
};

struct MatrixGroupRequest : public Request {
    MatrixGroupRequest() : Request(REQ_RES_COMMUNICATION_MATRIX_GROUP) {};
    MatrixGroupParam params;
};

struct MatrixBandwidthParam {
    std::string stage;
    std::string operatorName;
    std::string iterationId;
};

struct MatrixBandwidthRequest : public Request {
    MatrixBandwidthRequest() : Request(REQ_RES_COMMUNICATION_MATRIX_BANDWIDTH) {};
    MatrixBandwidthParam params;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_COMMUNICATION_PROTOCOL_COMMUNICATION_REQUEST_H