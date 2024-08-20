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
};

struct SummaryTopRankRequest : public Request {
    SummaryTopRankRequest() : Request(REQ_RES_SUMMARY_QUERY_TOP_DATA) {};
    SummaryTopRankParams params;
};

struct SummaryStatisticParams {
    std::string rankId;
    std::string timeFlag;
    std::string stepId;
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
};

struct PipelineStageRequest : public Request {
    PipelineStageRequest() : Request(REQ_RES_PIPELINE_GET_ALL_STAGES) {};
    PipelineStageParam params;
};

struct PipelineStageTimeParam {
    std::string stepId;
    std::string stageId;
};

struct PipelineStageTimeRequest : public Request {
    PipelineStageTimeRequest() : Request(REQ_RES_PIPELINE_STAGE_BUBBLE) {};
    PipelineStageTimeParam params;
};

struct PipelineRankTimeParam {
    std::string stepId;
    std::string stageId;
};

struct PipelineRankTimeRequest : public Request {
    PipelineRankTimeRequest() : Request(REQ_RES_PIPELINE_RANK_BUBBLE) {};
    PipelineRankTimeParam params;
};

struct ComputeDetailParams {
    std::string rankId;
    std::string timeFlag;
    int64_t currentPage = 0;
    int64_t pageSize = 0;
    std::string orderBy;
    std::string order;
    bool CommonCheck(std::string &errorMsg)
    {
        return CheckPageValid(this->pageSize, this->currentPage, errorMsg);
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
    bool CommonCheck(std::string &errorMsg)
    {
        return CheckPageValid(this->pageSize, this->currentPage, errorMsg);
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