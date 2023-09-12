/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_TIMELINE_PROTOCOL_UTIL_H
#define PROFILER_SERVER_TIMELINE_PROTOCOL_UTIL_H

#include <optional>
#include "GlobalDefs.h"
#include "TimelineProtocolResponse.h"
#include "TimelineProtocolEvent.h"

namespace Dic {
namespace Protocol {
// response
template <typename RESPONSE> std::optional<json_t> ToResponseJson(const RESPONSE &response);
template <> std::optional<json_t> ToResponseJson<ImportActionResponse>(const ImportActionResponse &response);
template <> std::optional<json_t> ToResponseJson<UnitThreadTracesResponse>(const UnitThreadTracesResponse &response);
template <> std::optional<json_t> ToResponseJson<UnitThreadsResponse>(const UnitThreadsResponse &response);
template <> std::optional<json_t> ToResponseJson<UnitThreadDetailResponse>(const UnitThreadDetailResponse &response);
template <> std::optional<json_t> ToResponseJson<UnitFlowNameResponse>(const UnitFlowNameResponse &response);
template <> std::optional<json_t> ToResponseJson<UnitFlowResponse>(const UnitFlowResponse &response);
template <> std::optional<json_t> ToResponseJson<ResetWindowResponse>(const ResetWindowResponse &response);
template <> std::optional<json_t> ToResponseJson<UnitChartResponse>(const UnitChartResponse &response);
template <> std::optional<json_t> ToResponseJson<SearchCountResponse>(const SearchCountResponse &response);
template <> std::optional<json_t> ToResponseJson<SearchSliceResponse>(const SearchSliceResponse &response);
// event
template <typename EVENT> std::optional<json_t> ToEventJson(const EVENT &event);
template <> std::optional<json_t> ToEventJson<ParseSuccessEvent>(const ParseSuccessEvent &event);
} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_TIMELINE_PROTOCOL_UTIL_H
