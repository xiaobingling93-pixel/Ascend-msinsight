/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_IEPROTOCOLUTIL_H
#define PROFILER_SERVER_IEPROTOCOLUTIL_H
#include <optional>
#include "GlobalDefs.h"
#include "IEProtocolEvent.h"
#include "IEProtocolResponse.h"

namespace Dic {
namespace Protocol {
// response
template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response);

template <> std::optional<document_t> ToResponseJson<IEUsageViewResponse>(const IEUsageViewResponse &response);
template <> std::optional<document_t> ToResponseJson<IETableViewResponse>(const IETableViewResponse &response);
template <> std::optional<document_t> ToResponseJson<IEGroupResponse>(const IEGroupResponse &response);
// event
template <typename EVENT> std::optional<document_t> ToEventJson(const EVENT &event);
template <>
std::optional<document_t> ToEventJson<ParseStatisticCompletedEvent>(const ParseStatisticCompletedEvent &event);
}
}
#endif // PROFILER_SERVER_IEPROTOCOLUTIL_H
