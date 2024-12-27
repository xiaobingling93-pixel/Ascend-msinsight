/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef DIC_COMMUNICATION_PROTOCOL_COMMUNICATION_RESPONSE_H
#define DIC_COMMUNICATION_PROTOCOL_COMMUNICATION_RESPONSE_H

#include <vector>
#include <cfloat>
#include "GlobalDefs.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
struct OperatorTimeItem {
    std::string operatorName;
    uint64_t startTime;
    uint64_t elapseTime;
};

struct OperatorItem {
    std::string operatorName;
    double startTime;
    double elapseTime;
    double transitTime;
    double synchronizationTime;
    double waitTime;
    double idleTime;
    double synchronizationTimeRatio;
    double waitTimeRatio;
    double sdmaBw{};
    double rdmaBw{};
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
    CompareData<std::vector<IterationsOrRanksObject>> body;
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

struct DurationData {
    double startTime;
    double elapseTime;
    double transitTime;
    double synchronizationTime;
    double waitTime;
    double idleTime;
    double synchronizationTimeRatio;
    double waitTimeRatio;
    double sdmaBw{};
    double rdmaBw{};
    double sdmaTime{};
    double rdmaTime{};

    DurationData operator-(const DurationData &durationData) const
    {
        DurationData res;
        res.startTime = this->startTime - durationData.startTime;
        res.elapseTime = this->elapseTime - durationData.elapseTime;
        res.transitTime = this->transitTime - durationData.transitTime;
        res.synchronizationTime = this->synchronizationTime - durationData.synchronizationTime;
        res.waitTime = this->waitTime - durationData.waitTime;
        res.idleTime = this->idleTime - durationData.idleTime;
        res.synchronizationTimeRatio = this->synchronizationTimeRatio - durationData.synchronizationTimeRatio;
        res.waitTimeRatio = this->waitTimeRatio - durationData.waitTimeRatio;
        res.sdmaBw = this->sdmaBw - durationData.sdmaBw;
        res.rdmaBw = this->rdmaBw - durationData.rdmaBw;
        res.sdmaTime = this->sdmaTime - durationData.sdmaTime;
        res.rdmaTime = this->rdmaTime - durationData.rdmaTime;
        return res;
    }
};

struct Duration {
    std::string rankId;
    CompareData<DurationData> durationData;
};

struct BandwidthStatistic {
    std::string type; // SDMA、RDMA
    double avgBw; // 单位GB/s
    double maxBw; // 单位GB/s
    double minBw; // 单位GB/s
    double diffBw; // 单位GB/s
    double allTime;
};

struct DurationListsResponseBody {
    std::vector<Duration> durationList;
    std::vector<BandwidthStatistic> bwStatistics{};
};

struct DurationResponse : public Response {
    DurationResponse() : Response(REQ_RES_COMMUNICATION_LIST) {}
    DurationListsResponseBody body;
};

struct OperatorListsResponseBody {
    uint64_t minTime = UINT64_MAX;
    uint64_t maxTime = 0;
    std::vector<std::string> rankLists;
    std::vector<CompareData<std::vector<OperatorTimeItem>>> opLists;
};

struct OperatorListsResponse : public Response {
    OperatorListsResponse() : Response(REQ_RES_COMMUNICATION_OPERATOR_LISTS) {}
    OperatorListsResponseBody body;
};

struct MatrixData {
    std::string transportType;
    std::string opName;
    double transitSize;
    double transitTime;
    double bandwidth;
};

struct MatrixList {
    int srcRank;
    int dstRank;
    CompareData<MatrixData> matrixData;
};

struct MatrixListResponseBody {
    std::vector<MatrixList> matrixList;
};

struct MatrixListResponse : public Response {
    MatrixListResponse() : Response(REQ_RES_COMMUNICATION_MATRIX_BANDWIDTH) {}

    MatrixListResponseBody body;
};

struct GroupInfo {
    std::string group;
    std::string parallelStrategy;
    std::string type;
};

struct MatrixGroupResponseBody {
    std::vector<GroupInfo> groupList;
};

struct MatrixGroupResponse : public Response {
    MatrixGroupResponse() : Response(REQ_RES_COMMUNICATION_MATRIX_GROUP) {}

    MatrixGroupResponseBody body;
};

} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_COMMUNICATION_PROTOCOL_COMMUNICATION_RESPONSE_H
