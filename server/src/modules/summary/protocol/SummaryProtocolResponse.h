/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 */

#ifndef DIC_TIMELINE_PROTOCOL_SUMMARY_RESPONSE_H
#define DIC_TIMELINE_PROTOCOL_SUMMARY_RESPONSE_H

#include <vector>
#include "GlobalDefs.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
struct SummaryDto {
    std::string rankId;
    double totalTime;
    double computingTime;
    double communicationOverLappedTime;
    double communicationNotOverLappedTime;
    double freeTime;
};

struct SummaryTopRankResBody {
    int rankCount;
    std::vector<std::string> rankList;
    double dataSize;
    long collectStartTime;
    std::string filePath;
    double collectDuration;
    int stepNum;
    std::vector<std::string> stepList;
    std::vector<SummaryDto> summaryList;
};

struct SummaryTopRankResponse : public Response {
    SummaryTopRankResponse() : Response(REQ_RES_SUMMARY_STATISTIC) {}

    SummaryTopRankResBody body;
};


struct SummaryStatisticsItem {
    std::string acceleratorCore;
    std::string overlapType;
    double duration;
    double utilization;
};
struct SummaryStatisticsBody {
    std::vector<SummaryStatisticsItem> summaryStatisticsItemList;
};
struct SummaryStatisticsResponse : public Response {
    SummaryStatisticsResponse() : Response(REQ_RES_SUMMARY_STATISTIC) {}

    SummaryStatisticsBody body;
};

struct ComputeDetail {
    std::string name;
    std::string type;
    double startTime;
    double duration;
    double waitTime;
    double blockDim;
    std::string inputShapes;
    std::string inputDataTypes;
    std::string inputFormats;
    std::string outputShapes;
    std::string outputDataTypes;
    std::string outputFormats;
};

struct ComputeDetailResponse : public Response {
    ComputeDetailResponse() : Response(REQ_RES_COMPUTE_DETAIL) {}
    std::vector<ComputeDetail> computeDetails;
    int64_t totalNum;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_TIMELINE_PROTOCOL_SUMMARY_RESPONSE_H
