/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_RLPROTOCOLUTIL_H
#define PROFILER_SERVER_RLPROTOCOLUTIL_H

#include <optional>
#include "GlobalDefs.h"
#include "RLProtocolResponse.h"

namespace Dic::Protocol {
// response
template<typename RESPONSE>
std::optional<document_t> ToResponseJson(const RESPONSE &response);

template<>
std::optional<document_t> ToResponseJson<RLPipelineResponse>(const RLPipelineResponse &response);
}

#endif // PROFILER_SERVER_RLPROTOCOLUTIL_H
