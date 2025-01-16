/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef DIC_SUMMARY_PROTOCOL_SUMMARY_REQUEST_H
#define DIC_SUMMARY_PROTOCOL_SUMMARY_REQUEST_H

#include <string>
#include <optional>
#include <vector>
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"
#include "ClusterDef.h"

namespace Dic {
namespace Protocol {
struct SummaryTopRankParams {
    int limit{};
    std::vector<std::string> stepIdList;
    std::vector<std::string> rankIdList;
    std::string orderBy;
    bool isCompare = false;
    bool CheckParams(std::string &errorMsg) const
    {
        std::string paramError;
        if (!CheckStrParamValid(this->orderBy, paramError)) {
            errorMsg = "[Summary] Failed to check orderBy." + paramError;
            return false;
        }
        return true;
    }
};

struct SummaryTopRankRequest : public Request {
    SummaryTopRankRequest() : Request(REQ_RES_SUMMARY_QUERY_TOP_DATA) {};
    SummaryTopRankParams params;
};

struct SummaryStatisticParams {
    std::string rankId;
    std::string timeFlag;
    std::string stepId;
    bool CheckParams(std::string &errorMsg) const
    {
        std::string paramError;
        if (!CheckStrParamValid(this->rankId, paramError)) {
            errorMsg = "[Summary] Failed to check rank id." + paramError;
            return false;
        }
        if (!CheckStrParamValid(this->timeFlag, paramError)) {
            errorMsg = "[Summary] Failed to check time flag." + paramError;
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(this->stepId, paramError)) {
            errorMsg = "[Summary] Failed to check step id." + paramError;
            return false;
        }
        return true;
    }
};

struct SummaryStatisticRequest : public Request {
    SummaryStatisticRequest() : Request(REQ_RES_SUMMARY_STATISTIC) {};
    SummaryStatisticParams params;
};

struct PipelineStepParam {
};

struct PipelineStepRequest : public Request {
    PipelineStepRequest() : Request(REQ_RES_PIPELINE_GET_ALL_STEPS) {};
    PipelineStepParam params;
};

struct PipelineStageParam {
    std::string stepId;
    bool CheckParams(std::string &errorMsg) const
    {
        std::string paramError;
        if (!CheckStrParamValidEmptyAllowed(this->stepId, paramError)) {
            errorMsg = "[Summary] Failed to check step id." + paramError;
            return false;
        }
        return true;
    }
};

struct PipelineStageRequest : public Request {
    PipelineStageRequest() : Request(REQ_RES_PIPELINE_GET_ALL_STAGES) {};
    PipelineStageParam params;
};

struct PipelineStageTimeParam {
    std::string stepId;
    std::string stageId;
    bool CheckParams(std::string &errorMsg) const
    {
        std::string paramError;
        if (!CheckStrParamValidEmptyAllowed(this->stepId, paramError)) {
            errorMsg = "[Summary] Failed to check step id." + paramError;
            return false;
        }
        if (!CheckStrParamValidWithoutLenLimit(this->stageId, paramError)) {
            errorMsg = "[Summary] Failed to check stage id." + paramError;
            return false;
        }
        return true;
    }
};

struct PipelineStageTimeRequest : public Request {
    PipelineStageTimeRequest() : Request(REQ_RES_PIPELINE_STAGE_BUBBLE) {};
    PipelineStageTimeParam params;
};

struct PipelineRankTimeParam {
    std::string stepId;
    std::string stageId;
    bool CheckParams(std::string &errorMsg) const
    {
        std::string paramError;
        if (!CheckStrParamValidEmptyAllowed(this->stepId, paramError)) {
            errorMsg = "[Summary] Failed to check step id." + paramError;
            return false;
        }
        if (!CheckStrParamValidWithoutLenLimit(this->stageId, paramError)) {
            errorMsg = "[Summary] Failed to check stage id." + paramError;
            return false;
        }
        return true;
    }
};

struct PipelineRankTimeRequest : public Request {
    PipelineRankTimeRequest() : Request(REQ_RES_PIPELINE_RANK_BUBBLE) {};
    PipelineRankTimeParam params;
};

struct PipelineFwdBwdTimelineRequest : public Request {
    PipelineFwdBwdTimelineRequest() : Request(REQ_RES_PIPELINE_FWD_BWD_TIMELINE) {};
    PipelineRankTimeParam params;
};

struct ParallelismArrangement {
    Module::ParallelStrategyConfig config;
    std::string dimension;
};

struct QueryParallelismArrangementRequest : public Request {
    QueryParallelismArrangementRequest() : Request(REQ_RES_PARALLELISM_ARRANGEMENT_ALL) {};
    ParallelismArrangement params;
};

struct ParallelismPerformance {
    Module::ParallelStrategyConfig config;
    std::string dimension;
    std::string orderBy;
    std::string step;
    bool isCompare = false;
    std::string baselineStep;
    std::vector<uint32_t> indexList;
};

struct QueryParallelismPerformanceRequest : public Request {
    QueryParallelismPerformanceRequest() : Request(REQ_RES_PARALLELISM_PERFORMANCE_DATA) {};
    ParallelismPerformance params;
};


const std::string KEY_ORDERBY = "orderBy";
const std::string KEY_STEP = "step";
const std::string KEY_IS_COMPARE = "isCompare";
const std::string KEY_BASELINE_STEP = "baselineStep";

struct ComputeDetailParams {
    std::string rankId;
    std::string timeFlag;
    int64_t currentPage = 0;
    int64_t pageSize = 0;
    std::string orderBy;
    std::string order;
    bool CheckParams(std::string &errorMsg) const
    {
        if (!CheckPageValid(this->pageSize, this->currentPage, errorMsg)) {
            return false;
        }
        std::string paramError;
        if (!CheckStrParamValid(this->timeFlag, paramError)) {
            errorMsg = "[Summary] Failed to check time flag." + paramError;
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(this->orderBy, paramError)) {
            errorMsg = "[Summary] Failed to check orderBy." + paramError;
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(this->order, paramError)) {
            errorMsg = "[Summary] Failed to check order." + paramError;
            return false;
        }
        return true;
    }
};

struct ComputeDetailRequest : public Request {
    ComputeDetailRequest() : Request(REQ_RES_COMPUTE_DETAIL) {};
    ComputeDetailParams params;
};

struct CommunicationDetailParams {
    std::string rankId;
    std::string timeFlag = "HCCL";
    int64_t currentPage = 0;
    int64_t pageSize = 0;
    std::string orderBy;
    std::string order;
    bool CheckParams(std::string &errorMsg) const
    {
        if (!CheckPageValid(this->pageSize, this->currentPage, errorMsg)) {
            return false;
        }
        std::string paramError;
        if (!CheckStrParamValid(this->rankId, paramError)) {
            errorMsg = "[Summary] Failed to check time flag." + paramError;
            return false;
        }
        if (!CheckStrParamValid(this->timeFlag, paramError)) {
            errorMsg = "[Summary] Failed to check rank id." + paramError;
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(this->orderBy, paramError)) {
            errorMsg = "[Summary] Failed to check orderBy." + paramError;
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(this->order, paramError)) {
            errorMsg = "[Summary] Failed to check order." + paramError;
            return false;
        }
        return true;
    }
};

struct CommunicationDetailRequest : public Request {
    CommunicationDetailRequest() : Request(REQ_RES_COMMUNICATION_DETAIL) {};
    CommunicationDetailParams params;
};

struct QueryParallelStrategyRequest : public Request {
    QueryParallelStrategyRequest() : Request(REQ_RES_SUMMARY_QUERY_PARALLEL_STRATEGY) {};
};

struct SetParallelStrategyRequest : public Request {
    SetParallelStrategyRequest() : Request(REQ_RES_SUMMARY_SET_PARALLEL_STRATEGY) {};
    Module::ParallelStrategyConfig config;
};

} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_SUMMARY_PROTOCOL_SUMMARY_REQUEST_H