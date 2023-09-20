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
    template<typename RESPONSE>std::optional<json_t> ToResponseJson(const RESPONSE &response);
    template<>std::optional<json_t> ToResponseJson<MemoryOperatorResponse>(const MemoryOperatorResponse &response);
    template<>std::optional<json_t> ToResponseJson<MemoryViewResponse>(const MemoryViewResponse &response);
    template<>
    std::optional<json_t> ToResponseJson<MemoryOperatorSizeResponse>(const MemoryOperatorSizeResponse &response);

} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_MEMORYPROTOCOLUTIL_H
