/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_GLOBAL_PROTOCOL_UTIL_H
#define PROFILER_SERVER_GLOBAL_PROTOCOL_UTIL_H

#include <optional>
#include "GlobalDefs.h"
#include "ProtocolResponse.h"

namespace Dic {
namespace Protocol {
// response
template <typename RESPONSE> std::optional<json_t> ToResponseJson(const RESPONSE &response);
template <> std::optional<json_t> ToResponseJson<TokenCreateResponse>(const TokenCreateResponse &response);
template <> std::optional<json_t> ToResponseJson<TokenDestroyResponse>(const TokenDestroyResponse &response);
template <> std::optional<json_t> ToResponseJson<TokenCheckResponse>(const TokenCheckResponse &response);
template <> std::optional<json_t> ToResponseJson<ConfigGetResponse>(const ConfigGetResponse &response);
template <> std::optional<json_t> ToResponseJson<ConfigSetResponse>(const ConfigSetResponse &response);
} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_GLOBAL_PROTOCOL_UTIL_H
