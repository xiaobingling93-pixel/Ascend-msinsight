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
#include "TimelineProtocolResponse.h"

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

struct SummaryBaseInfo {
    int rankCount = 0;
    std::vector<std::string> rankList;
    double dataSize = 0;
    int64_t collectStartTime = 0;
    std::string filePath;
    double collectDuration = 0;
    int stepNum = 0;
    std::vector<std::string> stepList;
};

struct SummaryTopRankResBody {
    CompareData<SummaryBaseInfo> baseInfo;
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

struct PipelineFwdBwdTimelineByComponent {
    std::string component; // FWD/BWD, P2P
    std::vector<Protocol::ThreadTraces> traceList;
};

struct PipelineFwdBwdTimelineByRank {
    std::string rankId;
    std::vector<std::string> componentList; // FWD/BWD, P2P
    std::vector<PipelineFwdBwdTimelineByComponent> componentDataList;
};

struct PipelineFwdBwdTimelineResponseBody {
    uint64_t minTime = UINT64_MAX;
    uint64_t maxTime = 0;
    std::vector<std::string> rankLists;
    std::vector<PipelineFwdBwdTimelineByRank> rankDataList;
};

struct PipelineFwdBwdTimelineResponse : public Response {
    PipelineFwdBwdTimelineResponse() : Response(REQ_RES_PIPELINE_FWD_BWD_TIMELINE) {}
    PipelineFwdBwdTimelineResponseBody body;
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

const std::string KEY_ALL = "all";
const std::string KEY_ALGORITHM = "algorithm";
const std::string KEY_DIMENSION = "dimension";
const std::string KEY_LEVEL = "level";
const std::string KEY_WORLD_SIZE = "worldSize";
const std::string KEY_TP_SIZE = "tpSize";
const std::string KEY_PP_SIZE = "ppSize";
const std::string KEY_DP_SIZE = "dpSize";
const std::string KEY_CP_SIZE = "cpSize";
const std::string KEY_EP_SIZE = "epSize";
const std::string KEY_MOE_TP_SIZE = "moeTpSize";
const std::string KEY_RESULT = "result";
const std::string KEY_MSG = "msg";
const std::string KEY_TP_INDEX = "tpIndex";
const std::string KEY_PP_INDEX = "ppIndex";
const std::string KEY_DP_INDEX = "dpIndex";
const std::string KEY_CP_INDEX = "cpIndex";
const std::string KEY_EP_INDEX = "epIndex";

struct QueryParallelStrategyResponse : public Response {
    QueryParallelStrategyResponse() : Response(REQ_RES_SUMMARY_QUERY_PARALLEL_STRATEGY) {}
    Module::ParallelStrategyConfig config;
    std::string level;
    const int64_t validValue = 1;
    bool IsValid() const
    {
        if (config.ppSize < validValue || config.tpSize < validValue || config.dpSize < validValue) {
            return false;
        }
        if (config.cpSize < validValue || config.epSize < validValue || config.moeTpSize < validValue) {
            return false;
        }
        if (std::find(Module::ALGORITHMS_ALLOWED.begin(), Module::ALGORITHMS_ALLOWED.end(), config.algorithm) ==
            Module::ALGORITHMS_ALLOWED.end()) {
            return false;
        }
        return true;
    }

    void SetDefault()
    {
        config.tpSize = std::max(validValue, config.tpSize);
        config.dpSize = std::max(validValue, config.dpSize);
        config.ppSize = std::max(validValue, config.ppSize);
        config.cpSize = std::max(validValue, config.cpSize);
        config.epSize = std::max(validValue, config.epSize);
        config.moeTpSize = std::max(validValue, config.moeTpSize);
        if (std::find(Module::ALGORITHMS_ALLOWED.begin(), Module::ALGORITHMS_ALLOWED.end(), config.algorithm) !=
            Module::ALGORITHMS_ALLOWED.end()) {
            return;
        }
        config.algorithm = Module::MEGATRON_LM_TP_CP_EP_DP_PP_ALG;
    }
};

struct SetParallelStrategyResponse : public Response {
    SetParallelStrategyResponse() : Response(REQ_RES_SUMMARY_SET_PARALLEL_STRATEGY) {}
    bool result = true;
    std::string msg;
};

struct ImportExpertDataResponse : public Response {
    ImportExpertDataResponse() : Response(REQ_RES_IMPORT_EXPERT_DATA) {}
    bool result = true;
    std::string msg;
};

struct QueryExpertHotspotBody {
    std::vector<Module::ExpertHotspotStruct> hotspotInfos;
};

struct QueryExpertHotspotResponse : public Response {
    QueryExpertHotspotResponse() : Response(REQ_RES_QUERY_EXPERT_HOTSPOT) {}
    QueryExpertHotspotBody body;
};

struct QueryModelInfoBody {
    int layerNum = 0;
    std::vector<int> denseLayerList;
    int expertNum = 0;
};

struct QueryModelInfoResponse : public Response {
    QueryModelInfoResponse() : Response(REQ_RES_QUERY_MODEL_INFO) {}
    QueryModelInfoBody body;
};

struct SummarySlowRankAdvisorBody {
    bool hasSlowRank = true;
    bool matchSuccess = true;
    std::vector<Module::AdviceInfoForSlowRank> topNElements;
};

struct SummarySlowRankAdvisorResponse : public Response {
    SummarySlowRankAdvisorResponse() : Response(REQ_RES_SUMMARY_SLOW_RANK_ADVISOR) {}
    SummarySlowRankAdvisorBody body;
};

struct ParallelismArrangementResponse : public Response {
    ParallelismArrangementResponse() : Response(REQ_RES_PARALLELISM_ARRANGEMENT_ALL) {}
    Module::ArrangementAndConnectionData arrangeData;
};

struct IndicatorDataStructVo {
    uint32_t index{};
    CompareData<std::unordered_map<std::string, double>> indicators;
    // 根据不同并行策略拆分的指标：实际数据根据通信耗时表计算得到
    CompareData<std::unordered_map<std::string, double>> commTimeIndicator;
};

struct PerformanceIndicatorData {
    std::vector<Module::IndicatorAttr> indicators;
    std::vector<IndicatorDataStructVo> performanceData;
    std::vector<std::string> advices;
};

struct ParallelismPerformanceResponse : public Response {
    ParallelismPerformanceResponse() : Response(REQ_RES_PARALLELISM_PERFORMANCE_DATA) {}
    PerformanceIndicatorData indicatorData;
};
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_TIMELINE_PROTOCOL_SUMMARY_RESPONSE_H
