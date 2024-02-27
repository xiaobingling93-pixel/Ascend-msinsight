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

struct CommunicatorGroupResponse : public Response {
    CommunicatorGroupResponse() : Response(REQ_RES_COMMUNICATION_COMMUNICATOR) {}
    rapidjson::Document body;
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


struct IterationsOrRanksObject {
    std::string iterationOrRankId;
};

struct IterationsOrRanksResponse : public Response {
    IterationsOrRanksResponse() : Response(REQ_RES_COMMUNICATION_ITERATIONS) {}
    std::vector<IterationsOrRanksObject> body;
};

struct RanksResponse : public Response {
    RanksResponse() : Response(REQ_RES_COMMUNICATION_RANKS) {}
    std::vector<IterationsOrRanksObject> body;
};

struct OperatorNamesObject {
    std::string operatorName;
};

struct OperatorNamesResponse : public Response {
    OperatorNamesResponse() : Response(REQ_RES_COMMUNICATION_OPERATORNAMES) {}
    std::vector<OperatorNamesObject> body;
};

struct MatrixSortOpNamesResponse : public Response {
    MatrixSortOpNamesResponse() : Response(REQ_RES_COMMUNICATION_SORT_OP) {}
    std::vector<OperatorNamesObject> body;
};

struct Duration {
    std::string rankId;
    double elapseTime;
    double transitTime;
    double synchronizationTime;
    double waitTime;
    double idleTime;
    double synchronizationTimeRatio;
    double waitTimeRatio;
};

struct DurationResponse : public Response {
    DurationResponse() : Response(REQ_RES_COMMUNICATION_LIST) {}
    std::vector<Duration> body;
};
struct MatrixList {
    int srcRank;
    int dstRank;
    std::string transportType;
    std::string opName;
    double transitSize;
    double transitTime;
    double bandwidth;
};

struct MatrixListResponseBody {
    std::vector<MatrixList> matrixList;
};

struct MatrixListResponse : public Response {
    MatrixListResponse() : Response(REQ_RES_COMMUNICATION_MATRIX_BANDWIDTH) {}

    MatrixListResponseBody body;
};

struct MatrixGroupResponseBody {
    std::vector<std::string> groupList;
};

struct MatrixGroupResponse : public Response {
    MatrixGroupResponse() : Response(REQ_RES_COMMUNICATION_MATRIX_GROUP) {}

    MatrixGroupResponseBody body;
};

} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_COMMUNICATION_PROTOCOL_COMMUNICATION_RESPONSE_H
