/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_OPERATORPROTOCOLUTIL_H
#define PROFILER_SERVER_OPERATORPROTOCOLUTIL_H

#include "GlobalDefs.h"
#include "OperatorProtocolEvent.h"
#include "OperatorProtocolResponse.h"

namespace Dic::Protocol {
template<typename RESPONSE>
std::optional<document_t> ToResponseJson(const RESPONSE &response);

template<>
std::optional<document_t> ToResponseJson<OperatorCategoryInfoResponse>(const OperatorCategoryInfoResponse &res);

template<>
std::optional<document_t> ToResponseJson<OperatorComputeUnitInfoResponse>(const OperatorComputeUnitInfoResponse &res);

template<>
std::optional<document_t> ToResponseJson<OperatorStatisticInfoResponse>(const OperatorStatisticInfoResponse &res);

template<>
std::optional<document_t> ToResponseJson<OperatorDetailInfoResponse>(const OperatorDetailInfoResponse &res);

template<>
std::optional<document_t> ToResponseJson<OperatorMoreInfoResponse>(const OperatorMoreInfoResponse &res);

template<typename EVENT>
std::optional<document_t> ToEventJson(const EVENT &event);

template<>
std::optional<document_t> ToEventJson<OperatorParseStatusEvent>(const OperatorParseStatusEvent &event);

template<>
std::optional<document_t> ToEventJson<OperatorParseClearEvent>(const OperatorParseClearEvent &event);
}

#endif // PROFILER_SERVER_OPERATORPROTOCOLUTIL_H
