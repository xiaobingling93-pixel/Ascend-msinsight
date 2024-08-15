/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_SUMMARY_PROTOCOL_UTIL_H
#define PROFILER_SERVER_SUMMARY_PROTOCOL_UTIL_H

#include <optional>
#include "GlobalDefs.h"
#include "SummaryProtocolResponse.h"

namespace Dic {
namespace Protocol {
// response
template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response);
template <> std::optional<document_t> ToResponseJson<SummaryTopRankResponse>(const SummaryTopRankResponse &response);
template <>
std::optional<document_t> ToResponseJson<SummaryStatisticsResponse>(const SummaryStatisticsResponse &response);
template <> std::optional<document_t> ToResponseJson<ComputeDetailResponse>(const ComputeDetailResponse &response);
template <> std::optional<document_t> ToResponseJson<PipelineStepResponse>(const PipelineStepResponse &response);
template <> std::optional<document_t> ToResponseJson<PipelineStageResponse>(const PipelineStageResponse &response);
template <>
std::optional<document_t> ToResponseJson<PipelineStageTimeResponse>(const PipelineStageTimeResponse &response);
template <>
std::optional<document_t> ToResponseJson<PipelineRankTimeResponse>(const PipelineRankTimeResponse &response);
template <>
std::optional<document_t> ToResponseJson<CommunicationDetailResponse>(const CommunicationDetailResponse &response);
template <>
std::optional<document_t> ToResponseJson<QueryParallelStrategyResponse>(const QueryParallelStrategyResponse &response);
template <>
std::optional<document_t> ToResponseJson<SetParallelStrategyResponse>(const SetParallelStrategyResponse &response);
} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_SUMMARY_PROTOCOL_UTIL_H
