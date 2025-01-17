/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 */

#ifndef PROFILER_SERVER_SOURCE_PROTOCOL_UTIL_H
#define PROFILER_SERVER_SOURCE_PROTOCOL_UTIL_H

#include <optional>
#include "GlobalDefs.h"
#include "SourceProtocolResponse.h"

namespace Dic {
namespace Protocol {

// response
template<typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response);
template<> std::optional<document_t> ToResponseJson<SourceCodeFileResponse>(const SourceCodeFileResponse &response);
template<> std::optional<document_t> ToResponseJson<SourceApiLineResponse>(const SourceApiLineResponse &response);
template<> std::optional<document_t> ToResponseJson<SourceApiInstrResponse>(const SourceApiInstrResponse &response);
template<>
std::optional<document_t> ToResponseJson<SourceApiInstrDynamicResponse>(const SourceApiInstrDynamicResponse &response);
template<> std::optional<document_t> ToResponseJson<DetailsBaseInfoResponse>(const DetailsBaseInfoResponse &response);
template<> std::optional<document_t> ToResponseJson<DetailsLoadInfoResponse>(const DetailsLoadInfoResponse &response);
template<>
std::optional<document_t> ToResponseJson<DetailsMemoryGraphResponse>(const DetailsMemoryGraphResponse &response);
template<>
std::optional<document_t> ToResponseJson<DetailsMemoryTableResponse>(const DetailsMemoryTableResponse &response);

std::optional<document_t> DetailsBaseInfoToJson(const DetailsBaseInfoResBody &body, Document::AllocatorType &allocator);
std::optional<document_t> SubBlockUnitDataToJson(const SubBlockUnitData &data, Document::AllocatorType &allocator);
std::optional<document_t> SubBlockDataToJson(const SubBlockData &data, Document::AllocatorType &allocator);
std::optional<document_t> UtilizationRateToJson(const UtilizationRate &rate, Document::AllocatorType &allocator);
std::optional<document_t> UtilizationRateCompareToJson(const CompareData<UtilizationRate> &compareRate,
                                                       Document::AllocatorType &allocator);
std::optional<document_t> MemoryUnitToJson(const MemoryUnit &memoryUnit, Document::AllocatorType &allocator);
std::optional<document_t> L2CacheToJson(const L2Cache &l2Cache, Document::AllocatorType &allocator);
std::optional<document_t> CompareTableRowToJson(const std::vector<CompareData<TableRow>> &rows,
                                                Document::AllocatorType &allocator);
} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_SOURCE_PROTOCOL_UTIL_H
