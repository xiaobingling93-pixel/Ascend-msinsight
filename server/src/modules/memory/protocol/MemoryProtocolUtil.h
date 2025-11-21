/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORYPROTOCOLUTIL_H
#define PROFILER_SERVER_MEMORYPROTOCOLUTIL_H


#include <optional>
#include "GlobalDefs.h"
#include "MemoryProtocolRespose.h"

namespace Dic {
namespace Protocol {
    // response
    template<typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response);
    template<> std::optional<document_t> ToResponseJson<MemoryOperatorComparisonResponse>(
        const MemoryOperatorComparisonResponse &response);
    template<> std::optional<document_t> ToResponseJson<MemoryComponentComparisonResponse>(
        const MemoryComponentComparisonResponse &response);
    template<> std::optional<document_t> ToResponseJson<MemoryViewResponse>(const MemoryViewResponse &response);
    template<> std::optional<document_t> ToResponseJson<MemoryTypeResponse>(const MemoryTypeResponse &response);
    template<> std::optional<document_t> ToResponseJson<MemoryResourceTypeResponse>
            (const MemoryResourceTypeResponse &response);
    template<> std::optional<document_t> ToResponseJson<MemoryStaticOperatorGraphResponse>
            (const MemoryStaticOperatorGraphResponse &response);
    template<> std::optional<document_t> ToResponseJson<MemoryStaticOperatorListCompResponse>
            (const MemoryStaticOperatorListCompResponse &response);
    template<> std::optional<document_t> ToResponseJson<MemoryStaticOperatorSizeResponse>
            (const MemoryStaticOperatorSizeResponse &response);
    template<>
    std::optional<document_t> ToResponseJson<MemoryOperatorSizeResponse>(const MemoryOperatorSizeResponse &response);
    std::optional<document_t> ToMemoryOperatorJson(const MemoryOperator &op, Document::AllocatorType &allocator);
    std::optional<document_t> ToMemoryComponentJson(const MemoryComponent &component,
        Document::AllocatorType &allocator);
    std::optional<document_t> ToMemoryStaticOperatorJson(const StaticOperatorItem &op,
        Document::AllocatorType &allocator);

} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_MEMORYPROTOCOLUTIL_H
