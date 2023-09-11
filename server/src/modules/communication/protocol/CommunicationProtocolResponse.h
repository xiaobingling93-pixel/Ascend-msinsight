/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef DIC_COMMUNICATION_PROTOCOL_COMMUNICATION_RESPONSE_H
#define DIC_COMMUNICATION_PROTOCOL_COMMUNICATION_RESPONSE_H

#include <vector>
#include "GlobalDefs.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
struct OperatorItem {
    std::string operatorName;
    double elapseTime;
    double transitTime;
    double synchronizationTime;
    double waitTime;
    double idleTime;
    double synchronizationTimeRatio;
    double waitTimeRatio;
};

struct OperatorDetailsResBody {
    int count;
    int pageSize;
    int currentPage;
    std::vector<OperatorItem> allOperators;
};

struct OperatorDetailsResponse : public Response {
    OperatorDetailsResponse() : Response(REQ_RES_COMMUNICATION_OPERATOR_DETAILS) {}
    OperatorDetailsResBody body;
};

struct GroupItem {
    GroupItem(std::string name, std::vector<int> ranks, std::string value) : name(name), ranks(ranks), value(value) {}
    std::string name;
    std::vector<int> ranks;
    std::string value;
};

struct CommunicatorGroupResBody {
    int defaultPPSize;
    std::vector<GroupItem> ppGroups;
    std::vector<GroupItem> tpOrDpGroups;
};

struct CommunicatorGroupResponse : public Response {
    CommunicatorGroupResponse() : Response(REQ_RES_COMMUNICATOR_PARSE) {}
    CommunicatorGroupResBody body;
};

struct BandwidthDataItem {
    std::string transportType;
    double transitSize;
    double transitTime;
    double bandwidth;
    double largePacketRatio;
};

struct BandwidthDataResBody {
    std::vector<BandwidthDataItem> items;
};

struct BandwidthDataResponse : public Response {
    BandwidthDataResponse() : Response(REQ_RES_COMMUNICATION_BANDWIDTH) {}
    BandwidthDataResBody body;
};

struct DistributionResBody {
    std::string distributionData;
};

struct DistributionResponse : public Response {
    DistributionResponse() : Response(REQ_RES_COMMUNICATION_DISTRIBUTION) {}
    DistributionResBody body;
};

} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_COMMUNICATION_PROTOCOL_COMMUNICATION_RESPONSE_H
