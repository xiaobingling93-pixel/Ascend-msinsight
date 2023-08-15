/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_PROTOCOL_RESPONSE_UTIL_H
#define DATA_INSIGHT_CORE_PROTOCOL_RESPONSE_UTIL_H

#include <optional>
#include "GlobalDefs.h"
#include "ProtocolResponse.h"

namespace Dic {
namespace Protocol {
template <typename RESPONSE> std::optional<json_t> ToResponseJson(const RESPONSE &response);
// global
template <> std::optional<json_t> ToResponseJson<TokenCreateResponse>(const TokenCreateResponse &response);
template <> std::optional<json_t> ToResponseJson<TokenDestroyResponse>(const TokenDestroyResponse &response);
template <> std::optional<json_t> ToResponseJson<TokenCheckResponse>(const TokenCheckResponse &response);
template <> std::optional<json_t> ToResponseJson<ConfigGetResponse>(const ConfigGetResponse &response);
template <> std::optional<json_t> ToResponseJson<ConfigSetResponse>(const ConfigSetResponse &response);
// ascend
template <> std::optional<json_t> ToResponseJson<ImportActionResponse>(const ImportActionResponse &response);
template <> std::optional<json_t> ToResponseJson<UnitThreadTracesResponse>(const UnitThreadTracesResponse &response);
template <> std::optional<json_t> ToResponseJson<UnitThreadsResponse>(const UnitThreadsResponse &response);
template <> std::optional<json_t> ToResponseJson<UnitThreadDetailResponse>(const UnitThreadDetailResponse &response);
template <> std::optional<json_t> ToResponseJson<UnitFlowNameResponse>(const UnitFlowNameResponse &response);
template <> std::optional<json_t> ToResponseJson<UnitFlowResponse>(const UnitFlowResponse &response);
template <> std::optional<json_t> ToResponseJson<ResetWindowResponse>(const ResetWindowResponse &response);
template <> std::optional<json_t> ToResponseJson<UnitChartResponse>(const UnitChartResponse &response);
} // end of namespace Protocol
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_PROTOCOL_RESPONSE_UTIL_H
