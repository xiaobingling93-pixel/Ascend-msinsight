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
template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response);
template <>
std::optional<document_t> ToResponseJson<ImportActionResponse>(const ImportActionResponse &response);
template <>
std::optional<document_t> ToResponseJson<UnitThreadTracesResponse>(const UnitThreadTracesResponse &response);
template <> std::optional<document_t> ToResponseJson<UnitThreadTracesSummaryResponse>(
    const UnitThreadTracesSummaryResponse &response);
template <> std::optional<document_t> ToResponseJson<UnitThreadsResponse>(const UnitThreadsResponse &response);
template <>
std::optional<document_t> ToResponseJson<UnitThreadDetailResponse>(const UnitThreadDetailResponse &response);
template <> std::optional<document_t> ToResponseJson<UnitFlowsResponse>(const UnitFlowsResponse &response);
template <> std::optional<document_t> ToResponseJson<ResetWindowResponse>(const ResetWindowResponse &response);
template <> std::optional<document_t> ToResponseJson<SearchCountResponse>(const SearchCountResponse &response);
template <> std::optional<document_t> ToResponseJson<SearchSliceResponse>(const SearchSliceResponse &response);
template <> std::optional<document_t> ToResponseJson<RemoteDeleteResponse>(const RemoteDeleteResponse &response);
template <>
std::optional<document_t> ToResponseJson<FlowCategoryListResponse>(const FlowCategoryListResponse &response);
template <>
std::optional<document_t> ToResponseJson<FlowCategoryEventsResponse>(const FlowCategoryEventsResponse &response);
template <> std::optional<document_t> ToResponseJson<UnitCounterResponse>(const UnitCounterResponse &response);
template<> std::optional<document_t> ToResponseJson<SystemViewResponse>(const SystemViewResponse &response);
template<> std::optional<document_t> ToResponseJson<EventsViewResponse>(const EventsViewResponse &response);
template<> std::optional<document_t> ToResponseJson<KernelDetailsResponse>(const KernelDetailsResponse &response);
template<> std::optional<document_t> ToResponseJson<OneKernelResponse>(const OneKernelResponse &response);
template<> std::optional<document_t> ToResponseJson<UnitThreadsOperatorsResponse>
        (const UnitThreadsOperatorsResponse &response);
template<> std::optional<document_t> ToResponseJson<SearchAllSlicesResponse>(const SearchAllSlicesResponse &response);
template<> std::optional<document_t> ToResponseJson<ParseCardsResponse>(const ParseCardsResponse &response);
// event
template <typename EVENT> std::optional<document_t> ToEventJson(const EVENT &event);
template <> std::optional<document_t> ToEventJson<ParseSuccessEvent>(const ParseSuccessEvent &event);
template <> std::optional<document_t> ToEventJson<ParseFailEvent>(const ParseFailEvent &event);
template <> std::optional<document_t> ToEventJson<ParseClusterCompletedEvent>(const ParseClusterCompletedEvent &event);
template <> std::optional<document_t> ToEventJson<AllSuccessEvent>(const AllSuccessEvent &event);
template <> std::optional<document_t> ToEventJson<ParseMemoryCompletedEvent>(const ParseMemoryCompletedEvent &event);
template <> std::optional<document_t> ToEventJson<ModuleResetEvent>(const ModuleResetEvent &event);
template <> std::optional<document_t> ToEventJson<ParseProgressEvent>(const ParseProgressEvent &event);
} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_TIMELINE_PROTOCOL_UTIL_H
