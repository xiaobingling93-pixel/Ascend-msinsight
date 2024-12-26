/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "pch.h"
#include "SummaryProtocolUtil.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "SummaryProtocol.h"

namespace Dic {
namespace Protocol {
void SummaryProtocol::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_RES_SUMMARY_QUERY_TOP_DATA, ToTopNRequest);
    jsonToReqFactory.emplace(REQ_RES_SUMMARY_STATISTIC, ToStatisticsRequest);
    jsonToReqFactory.emplace(REQ_RES_COMPUTE_DETAIL, ToComputeDetailRequest);
    jsonToReqFactory.emplace(REQ_RES_PIPELINE_GET_ALL_STEPS, ToStepRequest);
    jsonToReqFactory.emplace(REQ_RES_PIPELINE_GET_ALL_STAGES, ToStagesRequest);
    jsonToReqFactory.emplace(REQ_RES_PIPELINE_STAGE_BUBBLE, ToStageTimeRequest);
    jsonToReqFactory.emplace(REQ_RES_PIPELINE_RANK_BUBBLE, ToRankTimeRequest);
    jsonToReqFactory.emplace(REQ_RES_COMMUNICATION_DETAIL, ToCommunicationRequest);
    jsonToReqFactory.emplace(REQ_RES_SUMMARY_QUERY_PARALLEL_STRATEGY, ToQueryParallelStrategyRequest);
    jsonToReqFactory.emplace(REQ_RES_SUMMARY_SET_PARALLEL_STRATEGY, ToSetParallelStrategyRequest);
    jsonToReqFactory.emplace(REQ_RES_PIPELINE_FWD_BWD_TIMELINE, ToQueryFwdBwdTimelineRequest);
    jsonToReqFactory.emplace(REQ_RES_PARALLELISM_ARRANGEMENT_ALL, ToQueryParallelismArrangementRequest);
    jsonToReqFactory.emplace(REQ_RES_PARALLELISM_PERFORMANCE_DATA, ToQueryParallelismPerformanceRequest);
}

void SummaryProtocol::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_RES_SUMMARY_QUERY_TOP_DATA, ToTopNResponse);
    resToJsonFactory.emplace(REQ_RES_SUMMARY_STATISTIC, ToStatisticsResponse);
    resToJsonFactory.emplace(REQ_RES_COMPUTE_DETAIL, ToComputeDetailResponse);
    resToJsonFactory.emplace(REQ_RES_PIPELINE_GET_ALL_STEPS, ToStepResponse);
    resToJsonFactory.emplace(REQ_RES_PIPELINE_GET_ALL_STAGES, ToStagesResponse);
    resToJsonFactory.emplace(REQ_RES_PIPELINE_STAGE_BUBBLE, ToStageTimeResponse);
    resToJsonFactory.emplace(REQ_RES_PIPELINE_RANK_BUBBLE, ToRankTimeResponse);
    resToJsonFactory.emplace(REQ_RES_COMMUNICATION_DETAIL, ToCommunicationResponse);
    resToJsonFactory.emplace(REQ_RES_SUMMARY_QUERY_PARALLEL_STRATEGY, ToQueryParallelStrategyResponse);
    resToJsonFactory.emplace(REQ_RES_SUMMARY_SET_PARALLEL_STRATEGY, ToSetParallelStrategyResponse);
    resToJsonFactory.emplace(REQ_RES_PIPELINE_FWD_BWD_TIMELINE, ToQueryFwdBwdTimelineResponse);
    resToJsonFactory.emplace(REQ_RES_PARALLELISM_ARRANGEMENT_ALL, ToQueryParallelismArrangementResponse);
    resToJsonFactory.emplace(REQ_RES_PARALLELISM_PERFORMANCE_DATA, ToQueryParallelismPerformanceResponse);
}

void SummaryProtocol::RegisterEventToJsonFuncs()
{
}

#pragma region <<Json To Request>>

std::unique_ptr<Request> SummaryProtocol::ToTopNRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<SummaryTopRankRequest> reqPtr = std::make_unique<SummaryTopRankRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info of topN request.";
        return nullptr;
    }
    if (json["params"].HasMember("stepIdList") && json["params"]["stepIdList"].IsArray()) {
        for (const auto &stepId : json["params"]["stepIdList"].GetArray()) {
            reqPtr->params.stepIdList.emplace_back(stepId.GetString());
        }
    }
    if (json["params"].HasMember("rankIdList") && json["params"]["rankIdList"].IsArray()) {
        for (const auto &rankId : json["params"]["rankIdList"].GetArray()) {
            reqPtr->params.rankIdList.emplace_back(rankId.GetString());
        }
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.limit, json["params"], "limit");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.isCompare, json["params"], "isCompare");
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToStatisticsRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<SummaryStatisticRequest> reqPtr = std::make_unique<SummaryStatisticRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info of statistics request.";
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.stepId, json["params"], "stepId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.timeFlag, json["params"], "timeFlag");
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToComputeDetailRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<ComputeDetailRequest> reqPtr = std::make_unique<ComputeDetailRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info of compute detail request.";
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.currentPage, json["params"], "currentPage");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.timeFlag, json["params"], "timeFlag");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToStepRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<PipelineStepRequest> reqPtr = std::make_unique<PipelineStepRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info of step request.";
        return nullptr;
    }
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToStagesRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<PipelineStageRequest> reqPtr = std::make_unique<PipelineStageRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info of stages request";
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.stepId, json["params"], "stepId");
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToStageTimeRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<PipelineStageTimeRequest> reqPtr = std::make_unique<PipelineStageTimeRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info of stage time request.";
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.stepId, json["params"], "stepId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.stageId, json["params"], "stageId");
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToRankTimeRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<PipelineRankTimeRequest> reqPtr = std::make_unique<PipelineRankTimeRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info of rank time request.";
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.stepId, json["params"], "stepId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.stageId, json["params"], "stageId");
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToCommunicationRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<CommunicationDetailRequest> reqPtr = std::make_unique<CommunicationDetailRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info of communication request.";
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.currentPage, json["params"], "currentPage");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.timeFlag, json["params"], "timeFlag");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.pageSize, json["params"], "pageSize");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.order, json["params"], "order");
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToQueryParallelStrategyRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<QueryParallelStrategyRequest> reqPtr = std::make_unique<QueryParallelStrategyRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info of query parallel strategy request.";
        return nullptr;
    }
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToSetParallelStrategyRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<SetParallelStrategyRequest> reqPtr = std::make_unique<SetParallelStrategyRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info of set parallel strategy request.";
        return nullptr;
    }
    std::vector<std::string> keys = {KEY_ALGORITHM, KEY_TP_SIZE, KEY_PP_SIZE, KEY_DP_SIZE};
    for (auto &item : keys) {
        if (!json["params"].HasMember(item.c_str())) {
            error = "Set parallel strategy request didn't have key: " + item;
            return nullptr;
        }
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->config.algorithm, json["params"], KEY_ALGORITHM);
    JsonUtil::SetByJsonKeyValue(reqPtr->config.tpSize, json["params"], KEY_TP_SIZE);
    JsonUtil::SetByJsonKeyValue(reqPtr->config.ppSize, json["params"], KEY_PP_SIZE);
    JsonUtil::SetByJsonKeyValue(reqPtr->config.dpSize, json["params"], KEY_DP_SIZE);
    if (json["params"].HasMember(KEY_CP_SIZE.c_str())) {
        JsonUtil::SetByJsonKeyValue(reqPtr->config.cpSize, json["params"], KEY_CP_SIZE);
    }
    if (json["params"].HasMember(KEY_EP_SIZE.c_str())) {
        JsonUtil::SetByJsonKeyValue(reqPtr->config.epSize, json["params"], KEY_EP_SIZE);
    }
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToQueryFwdBwdTimelineRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<PipelineFwdBwdTimelineRequest> reqPtr = std::make_unique<PipelineFwdBwdTimelineRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info of query fwd/bwd timeline request.";
        return nullptr;
    }
    if (!json.HasMember("params") || !json["params"].HasMember("stepId") || !json["params"].HasMember("stageId")) {
        error = "Failed to set request parameter of query fwd/bwd timeline request due to missing parameter.";
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.stepId, json["params"], "stepId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.stageId, json["params"], "stageId");
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToQueryParallelismArrangementRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<QueryParallelismArrangementRequest> reqPtr = std::make_unique<QueryParallelismArrangementRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info of query parallelism arrangement request.";
        return nullptr;
    }
    std::vector<std::string> keys = {KEY_ALGORITHM, KEY_TP_SIZE, KEY_PP_SIZE, KEY_DP_SIZE, KEY_CP_SIZE, KEY_EP_SIZE,
                                     KEY_DIMENSION};
    for (auto &item : keys) {
        if (!json["params"].HasMember(item.c_str())) {
            error = "Query parallelism arrangement request didn't have key: " + item;
            return nullptr;
        }
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.config.algorithm, json["params"], KEY_ALGORITHM);
    JsonUtil::SetByJsonKeyValue(reqPtr->params.config.tpSize, json["params"], KEY_TP_SIZE);
    JsonUtil::SetByJsonKeyValue(reqPtr->params.config.ppSize, json["params"], KEY_PP_SIZE);
    JsonUtil::SetByJsonKeyValue(reqPtr->params.config.dpSize, json["params"], KEY_DP_SIZE);
    JsonUtil::SetByJsonKeyValue(reqPtr->params.config.cpSize, json["params"], KEY_CP_SIZE);
    JsonUtil::SetByJsonKeyValue(reqPtr->params.config.epSize, json["params"], KEY_EP_SIZE);
    JsonUtil::SetByJsonKeyValue(reqPtr->params.dimension, json["params"], KEY_DIMENSION);
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToQueryParallelismPerformanceRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<QueryParallelismPerformanceRequest> reqPtr = std::make_unique<QueryParallelismPerformanceRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info of query parallelism performance request.";
        return nullptr;
    }
    std::vector<std::string> keys = {KEY_ALGORITHM, KEY_TP_SIZE, KEY_PP_SIZE, KEY_DP_SIZE, KEY_CP_SIZE, KEY_EP_SIZE,
                                     KEY_DIMENSION, KEY_STEP};
    for (auto &item : keys) {
        if (!json["params"].HasMember(item.c_str())) {
            error = "Query parallelism performance request didn't have key: " + item;
            return nullptr;
        }
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.config.algorithm, json["params"], KEY_ALGORITHM);
    JsonUtil::SetByJsonKeyValue(reqPtr->params.config.tpSize, json["params"], KEY_TP_SIZE);
    JsonUtil::SetByJsonKeyValue(reqPtr->params.config.ppSize, json["params"], KEY_PP_SIZE);
    JsonUtil::SetByJsonKeyValue(reqPtr->params.config.dpSize, json["params"], KEY_DP_SIZE);
    JsonUtil::SetByJsonKeyValue(reqPtr->params.config.cpSize, json["params"], KEY_CP_SIZE);
    JsonUtil::SetByJsonKeyValue(reqPtr->params.config.epSize, json["params"], KEY_EP_SIZE);
    JsonUtil::SetByJsonKeyValue(reqPtr->params.dimension, json["params"], KEY_DIMENSION);
    JsonUtil::SetByJsonKeyValue(reqPtr->params.step, json["params"], KEY_STEP);
    return reqPtr;
}

#pragma endregion

#pragma region <<Json To Request>>

std::optional<document_t> SummaryProtocol::ToTopNResponse(const Response &response)
{
    return ToResponseJson<SummaryTopRankResponse>(dynamic_cast<const SummaryTopRankResponse &>(response));
}

std::optional<document_t> SummaryProtocol::ToStatisticsResponse(const Response &response)
{
    return ToResponseJson<SummaryStatisticsResponse>(dynamic_cast<const SummaryStatisticsResponse &>(response));
}

std::optional<document_t> SummaryProtocol::ToComputeDetailResponse(const Response &response)
{
    return ToResponseJson<ComputeDetailResponse>(dynamic_cast<const ComputeDetailResponse &>(response));
}

std::optional<document_t> SummaryProtocol::ToStepResponse(const Response &response)
{
    return ToResponseJson<PipelineStepResponse>(dynamic_cast<const PipelineStepResponse &>(response));
}

std::optional<document_t> SummaryProtocol::ToStagesResponse(const Response &response)
{
    return ToResponseJson<PipelineStageResponse>(dynamic_cast<const PipelineStageResponse &>(response));
}

std::optional<document_t> SummaryProtocol::ToStageTimeResponse(const Response &response)
{
    return ToResponseJson<PipelineStageTimeResponse>(dynamic_cast<const PipelineStageTimeResponse &>(response));
}

std::optional<document_t> SummaryProtocol::ToRankTimeResponse(const Response &response)
{
    return ToResponseJson<PipelineRankTimeResponse>(dynamic_cast<const PipelineRankTimeResponse &>(response));
}

std::optional<document_t> SummaryProtocol::ToCommunicationResponse(const Response &response)
{
    return ToResponseJson<CommunicationDetailResponse>(dynamic_cast<const CommunicationDetailResponse &>(response));
}

std::optional<document_t> SummaryProtocol::ToQueryParallelStrategyResponse(const Response &response)
{
    return ToResponseJson<QueryParallelStrategyResponse>(dynamic_cast<const QueryParallelStrategyResponse &>(response));
}

std::optional<document_t> SummaryProtocol::ToSetParallelStrategyResponse(const Response &response)
{
    return ToResponseJson<SetParallelStrategyResponse>(dynamic_cast<const SetParallelStrategyResponse &>(response));
}

std::optional<document_t> SummaryProtocol::ToQueryFwdBwdTimelineResponse(const Response &response)
{
    return ToResponseJson<PipelineFwdBwdTimelineResponse>(
        dynamic_cast<const PipelineFwdBwdTimelineResponse &>(response));
}

std::optional<document_t> SummaryProtocol::ToQueryParallelismArrangementResponse(const Response &response)
{
    return ToResponseJson<ParallelismArrangementResponse>(
        dynamic_cast<const ParallelismArrangementResponse &>(response));
}

std::optional<document_t> SummaryProtocol::ToQueryParallelismPerformanceResponse(const Response &response)
{
    return ToResponseJson<ParallelismPerformanceResponse>(
        dynamic_cast<const ParallelismPerformanceResponse &>(response));
}
#pragma endregion
} // namespace Protocol
} // namespace Dic
