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
template <typename RESPONSE> std::optional<json_t> ToResponseJson(const RESPONSE &response);
template <> std::optional<json_t> ToResponseJson<SummaryTopRankResponse>(const SummaryTopRankResponse &response);
template <> std::optional<json_t> ToResponseJson<SummaryStatisticsResponse>(const SummaryStatisticsResponse &response);
template <> std::optional<json_t> ToResponseJson<ComputeDetailResponse>(const ComputeDetailResponse &response);
template <> std::optional<json_t> ToResponseJson<PipelineStepResponse>(const PipelineStepResponse &response);
template <> std::optional<json_t> ToResponseJson<PipelineStageResponse>(const PipelineStageResponse &response);
template <> std::optional<json_t> ToResponseJson<PipelineStageTimeResponse>(const PipelineStageTimeResponse &response);
template <> std::optional<json_t> ToResponseJson<PipelineRankTimeResponse>(const PipelineRankTimeResponse &response);
} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_SUMMARY_PROTOCOL_UTIL_H
