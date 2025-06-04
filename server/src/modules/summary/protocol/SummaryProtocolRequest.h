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
    bool isCompare = false;
    std::string clusterPath;
    inline bool CheckParams(std::string &errMsg) const
    {
        std::string paramError;
        if (!CheckStrParamValid(clusterPath, paramError)) {
            errMsg = "[Summary] Failed to check cluster";
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
    std::string clusterPath;

    inline bool CheckParams(std::string& errMsg) const
    {
        std::string paramErr;
        if (!CheckStrParamValid(clusterPath, paramErr)) {
            errMsg = "[Summary] Failed to check cluster" + paramErr;
            return false;
        }
        return true;
    }
};

struct PipelineStepRequest : public Request {
    PipelineStepRequest() : Request(REQ_RES_PIPELINE_GET_ALL_STEPS) {};
    PipelineStepParam params;
};

struct PipelineStageParam {
    std::string stepId;
    std::string clusterPath;
    bool CheckParams(std::string &errorMsg) const
    {
        std::string paramError;
        if (!CheckStrParamValidEmptyAllowed(this->stepId, paramError)) {
            errorMsg = "[Summary] Failed to check step id." + paramError;
            return false;
        }
        if (!CheckStrParamValid(clusterPath, paramError)) {
            errorMsg = "[Summary] Failed to check cluster." + paramError;
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
    std::string clusterPath;
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
        if (!CheckStrParamValid(clusterPath, paramError)) {
            errorMsg = "[Summary] Failed to check cluster." + paramError;
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
    std::string clusterPath;
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
        if (!CheckStrParamValid(clusterPath, paramError)) {
            errorMsg = "[Summary] Failed to check cluster." + paramError;
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
    std::string clusterPath;
    bool CheckParams(std::string &errorMsg) const
    {
        if (!config.CheckParams(errorMsg)) {
            return false;
        }
        if (std::find(Module::DIMENSIONS_ALLOWED.begin(), Module::DIMENSIONS_ALLOWED.end(), dimension) ==
            Module::DIMENSIONS_ALLOWED.end()) {
            errorMsg = "[Summary] Dimension is not allowed.";
            return false;
        }
        std::string paramErr;
        if (!CheckStrParamValid(clusterPath, paramErr)) {
            errorMsg = "[Summary] Failed to check cluster." + paramErr;
            return false;
        }
        return true;
    }
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
    std::string clusterPath;
    bool CheckParams(std::string &errorMsg) const
    {
        if (!config.CheckParams(errorMsg)) {
            return false;
        }
        if (std::find(Module::DIMENSIONS_ALLOWED.begin(), Module::DIMENSIONS_ALLOWED.end(), dimension) ==
            Module::DIMENSIONS_ALLOWED.end()) {
            errorMsg = "[Summary] Dimension is not allowed.";
            return false;
        }
        std::string paramError;
        if (!CheckStrParamValidEmptyAllowed(this->orderBy, paramError)) {
            errorMsg = "[Summary] Failed to check orderBy." + paramError;
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(this->step, paramError)) {
            errorMsg = "[Summary] Failed to check step." + paramError;
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(this->baselineStep, paramError)) {
            errorMsg = "[Summary] Failed to check baselineStep." + paramError;
            return false;
        }
        if (!CheckStrParamValid(clusterPath, paramError)) {
            errorMsg = "[Summary] Failed to check cluster." + paramError;
            return false;
        }
        return true;
    }
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
    std::string dbPath;
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
        if (!CheckStrParamValid(this->rankId, paramError)) {
            errorMsg = "[Summary] Failed to check rank id." + paramError;
            return false;
        }
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
            errorMsg = "[Summary] Failed to check rank id." + paramError;
            return false;
        }
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

struct CommunicationDetailRequest : public Request {
    CommunicationDetailRequest() : Request(REQ_RES_COMMUNICATION_DETAIL) {};
    CommunicationDetailParams params;
};

struct ParallelStrategyParam {
    std::string clusterPath;

    inline bool CheckParams(std::string &errMsg) const
    {
        std::string paramErr;
        if (!CheckStrParamValid(clusterPath, paramErr)) {
            errMsg = "[Summary] Failed to check cluster." + paramErr;
            return false;
        }
        return true;
    }
};

struct QueryParallelStrategyRequest : public Request {
    QueryParallelStrategyRequest() : Request(REQ_RES_SUMMARY_QUERY_PARALLEL_STRATEGY) {};
    ParallelStrategyParam params;
};

struct SetParallelStrategyParam {
    Module::ParallelStrategyConfig config;
    std::string clusterPath;
    bool CheckParams(std::string& errMsg) const
    {
        std::string paramErr;
        if (!config.CheckParams(paramErr)) {
            errMsg = "[Summary] Failed to check parallel strategy config." + paramErr;
            return false;
        }
        if (!CheckStrParamValid(clusterPath, paramErr)) {
            errMsg = "[Summary] Failed to check cluster." + paramErr;
            return false;
        }
        return true;
    }
};
struct SetParallelStrategyRequest : public Request {
    SetParallelStrategyRequest() : Request(REQ_RES_SUMMARY_SET_PARALLEL_STRATEGY) {};
    SetParallelStrategyParam params;
};

struct ImportExpertDataParams {
    std::string filePath;
    std::string version;
    std::string clusterPath;

    bool CheckParams(std::string &errorMsg) const
    {
        std::string paramError;
        if (!CheckStrParamValid(this->filePath, paramError)) {
            errorMsg = "[Summary] Failed to check file path." + paramError;
            return false;
        }
        if (!CheckStrParamValid(this->version, paramError)) {
            errorMsg = "[Summary] Failed to check version." + paramError;
            return false;
        }
        if (!CheckStrParamValid(clusterPath, paramError)) {
            errorMsg = "[Summary] Failed to check cluster." + paramError;
            return false;
        }
        return true;
    }
};

struct ImportExpertDataRequest : public Request {
    ImportExpertDataRequest() : Request(REQ_RES_IMPORT_EXPERT_DATA) {};
    ImportExpertDataParams params;
};

struct ModelInfoParam {
    std::string clusterPath;
    inline bool CheckParams(std::string& errMsg) const
    {
        std::string paramErr;
        if (!CheckStrParamValid(clusterPath, paramErr)) {
            errMsg = "[Summary] Failed to check cluster." + paramErr;
            return false;
        }
        return true;
    }
};

struct QueryModelInfoRequest : public Request {
    QueryModelInfoRequest() : Request(REQ_RES_QUERY_MODEL_INFO) {};
    ModelInfoParam params;
};

struct QueryExpertHotspotParams {
    std::string modelStage;
    std::string version;
    std::vector<int> denseLayerList;
    int layerNum = 0;
    int expertNum = 0;
    std::string clusterPath;

    bool CheckParams(std::string &errorMsg) const
    {
        std::string paramError;
        if (!CheckStrParamValid(this->modelStage, paramError)) {
            errorMsg = "[Summary] Failed to check model stage." + paramError;
            return false;
        }
        if (!CheckStrParamValid(this->version, paramError)) {
            errorMsg = "[Summary] Failed to check version." + paramError;
            return false;
        }
        if (layerNum <= 0 || expertNum <= 0) {
            errorMsg = "[Summary] The number of layer and the number of expert must be greater than zero.";
            return false;
        }
        for (const auto &item: denseLayerList) {
            if (item >= layerNum) {
                errorMsg = "[Summary] The range of dense layer is out of layer number.";
                return false;
            }
        }
        if (!CheckStrParamValid(clusterPath, paramError)) {
            errorMsg = "[Summary] Failed to check cluster." + paramError;
            return false;
        }
        return true;
    }
};

struct QueryExpertHotspotRequest : public Request {
    QueryExpertHotspotRequest() : Request(REQ_RES_QUERY_EXPERT_HOTSPOT) {};
    QueryExpertHotspotParams params;
};

} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_SUMMARY_PROTOCOL_SUMMARY_REQUEST_H