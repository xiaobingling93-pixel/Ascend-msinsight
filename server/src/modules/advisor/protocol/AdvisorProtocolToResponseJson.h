/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */


#ifndef PROFILER_SERVER_ADVISORPROTOCOLTORESPONSEJSON_H
#define PROFILER_SERVER_ADVISORPROTOCOLTORESPONSEJSON_H

#include <optional>
#include "GlobalDefs.h"
#include "AdvisorProtocolResponse.h"

namespace Dic::Protocol {
template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response);
template <>
std::optional<document_t> ToResponseJson<AffinityOptimizerResponse>(const AffinityOptimizerResponse &response);
template <> std::optional<document_t> ToResponseJson<AffinityAPIResponse>(const AffinityAPIResponse &response);
template <> std::optional<document_t> ToResponseJson<OperatorFusionResponse>(const OperatorFusionResponse &response);
template <> std::optional<document_t> ToResponseJson<AICpuOperatorResponse>(const AICpuOperatorResponse &response);
template <> std::optional<document_t> ToResponseJson<AclnnOperatorResponse>(const AclnnOperatorResponse &response);
template <> std::optional<document_t> ToResponseJson<OperatorDispatchResponse>(
    const OperatorDispatchResponse &response);

class AdvisorProtocolToResponseJson {
public:
    static std::optional<document_t> ToAffinityOptimizerResponse(const Response &response);
    static std::optional<document_t> ToAffinityAPIResponse(const Response &response);
    static std::optional<document_t> ToOperatorFusionResponse(const Response &response);
    static std::optional<document_t> ToAICpuOperatorResponse(const Response &response);
    static std::optional<document_t> ToAclnnOperatorResponse(const Response &response);
    static std::optional<document_t> ToOperatorDispatchResponse(const Response &response);
};
}

#endif // PROFILER_SERVER_ADVISORPROTOCOLTORESPONSEJSON_H
