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

} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_SOURCE_PROTOCOL_UTIL_H
