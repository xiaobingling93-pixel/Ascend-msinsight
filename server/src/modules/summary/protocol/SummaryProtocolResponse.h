/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 */

#ifndef DIC_TIMELINE_PROTOCOL_SUMMARY_RESPONSE_H
#define DIC_TIMELINE_PROTOCOL_SUMMARY_RESPONSE_H

#include <vector>
#include "GlobalDefs.h"
#include "ClusterDef.h"
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
    double prepareTime;
};

struct TraceStatistic {
    double computeDiff{};
    double communicationDiff{};
    double freeDiff{};
};

struct SummaryTopRankResBody {
    int rankCount;
    std::vector<std::string> rankList;
    double dataSize;
    int64_t collectStartTime;
    std::string filePath;
    double collectDuration;
    int stepNum;
    std::vector<std::string> stepList;
    std::vector<SummaryDto> summaryList;
    TraceStatistic traceStatistic;
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

struct PipelineStepResponseBody {
    std::vector<std::string> stepList;
};

struct PipelineStepResponse : public Response {
    PipelineStepResponse() : Response(REQ_RES_PIPELINE_GET_ALL_STEPS) {}

    PipelineStepResponseBody body;
};

struct PipelineStageResponseBody {
    std::vector<std::string> stageList;
};

struct PipelineStageResponse : public Response {
    PipelineStageResponse() : Response(REQ_RES_PIPELINE_GET_ALL_STAGES) {}

    PipelineStageResponseBody body;
};

struct BubbleDetail {
    std::string stageOrRankId;
    double stageTime;
    double bubbleTime;
};

struct PipelineStageOrRankTimeResponseBody {
    std::vector<BubbleDetail> bubbleDetails;
};

struct PipelineStageTimeResponse : public Response {
    PipelineStageTimeResponse() : Response(REQ_RES_PIPELINE_STAGE_BUBBLE) {}

    PipelineStageOrRankTimeResponseBody body;
};

struct PipelineRankTimeResponse : public Response {
    PipelineRankTimeResponse() : Response(REQ_RES_PIPELINE_RANK_BUBBLE) {}

    PipelineStageOrRankTimeResponseBody body;
};

struct SummaryStatisticsResponse : public Response {
    SummaryStatisticsResponse() : Response(REQ_RES_SUMMARY_STATISTIC) {}

    SummaryStatisticsBody body;
};

struct ComputeDetail {
    std::string name;
    std::string type;
    std::string startTime;
    double duration;
    double waitTime;
    int64_t blockDim;
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
    int64_t totalNum{};
};

struct CommunicationDetail {
    std::string name;
    std::string type;
    std::string startTime;
    double duration;
    double waitTime;
};

struct CommunicationDetailResponse : public Response {
    CommunicationDetailResponse() : Response(REQ_RES_COMMUNICATION_DETAIL) {}
    std::vector<CommunicationDetail> commDetails;
    int64_t totalNum{};
};

const std::string KEY_ALGORITHM = "algorithm";
const std::string KEY_LEVEL = "level";
const std::string KEY_WORLD_SIZE = "worldSize";
const std::string KEY_TP_SIZE = "tpSize";
const std::string KEY_PP_SIZE = "ppSize";
const std::string KEY_DP_SIZE = "dpSize";
const std::string KEY_RESULT = "result";
const std::string KEY_MSG = "msg";

struct QueryParallelStrategyResponse : public Response {
    QueryParallelStrategyResponse() : Response(REQ_RES_SUMMARY_QUERY_PARALLEL_STRATEGY) {}
    Module::ParallelStrategyConfig config;
    std::string level;
};

struct SetParallelStrategyResponse : public Response {
    SetParallelStrategyResponse() : Response(REQ_RES_SUMMARY_SET_PARALLEL_STRATEGY) {}
    bool result = true;
    std::string msg;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_TIMELINE_PROTOCOL_SUMMARY_RESPONSE_H
