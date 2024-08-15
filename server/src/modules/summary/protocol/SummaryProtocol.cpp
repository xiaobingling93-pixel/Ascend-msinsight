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
}

void SummaryProtocol::RegisterEventToJsonFuncs()
{
}

#pragma region <<Json To Request>>

std::unique_ptr<Request> SummaryProtocol::ToTopNRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<SummaryTopRankRequest> reqPtr = std::make_unique<SummaryTopRankRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
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
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToStatisticsRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<SummaryStatisticRequest> reqPtr = std::make_unique<SummaryStatisticRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
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
        error = "Failed to set request base info, command is: " + reqPtr->command;
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
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToStagesRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<PipelineStageRequest> reqPtr = std::make_unique<PipelineStageRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.stepId, json["params"], "stepId");
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToStageTimeRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<PipelineStageTimeRequest> reqPtr = std::make_unique<PipelineStageTimeRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
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
        error = "Failed to set request base info, command is: " + reqPtr->command;
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
        error = "Failed to set request base info, command is: " + reqPtr->command;
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
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToSetParallelStrategyRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<SetParallelStrategyRequest> reqPtr = std::make_unique<SetParallelStrategyRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
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
#pragma endregion
} // namespace Protocol
} // namespace Dic
