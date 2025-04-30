/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATION_PROTOCOL_UTIL_H
#define PROFILER_SERVER_COMMUNICATION_PROTOCOL_UTIL_H

#include <optional>
#include "CommunicationProtocolResponse.h"

namespace Dic {
namespace Protocol {
// response
template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response);
template <> std::optional<document_t> ToResponseJson<OperatorDetailsResponse>(const OperatorDetailsResponse &response);
template <> std::optional<document_t> ToResponseJson<DistributionResponse>(const DistributionResponse &response);
template <> std::optional<document_t> ToResponseJson<BandwidthDataResponse>(const BandwidthDataResponse &response);
template <>
std::optional<document_t> ToResponseJson<IterationsOrRanksResponse>(const IterationsOrRanksResponse &response);
template <> std::optional<document_t> ToResponseJson<OperatorNamesResponse>(const OperatorNamesResponse &response);
template <>
std::optional<document_t> ToResponseJson<MatrixSortOpNamesResponse>(const MatrixSortOpNamesResponse &response);
template <> std::optional<document_t> ToResponseJson<DurationResponse>(const DurationResponse &response);
template <> std::optional<document_t> ToResponseJson<OperatorListsResponse>(const OperatorListsResponse &response);
template <> std::optional<document_t> ToResponseJson<RanksResponse>(const RanksResponse &response);

template <> std::optional<document_t> ToResponseJson<MatrixGroupResponse>(const MatrixGroupResponse &response);
template <> std::optional<document_t> ToResponseJson<MatrixListResponse>(const MatrixListResponse &response);

template <> std::optional<document_t> ToResponseJson<CommunicationAdvisorResponse>(
    const CommunicationAdvisorResponse &response);

std::optional<document_t> MatrixDataToJson(const MatrixData &matrixData, Document::AllocatorType &allocator);

} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_COMMUNICATION_PROTOCOL_UTIL_H
