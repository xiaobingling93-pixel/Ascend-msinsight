/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_GLOBAL_PROTOCOL_UTIL_H
#define PROFILER_SERVER_GLOBAL_PROTOCOL_UTIL_H

#include <optional>
#include "GlobalProtocolResponse.h"
#include "GlobalProtocolEvent.h"

namespace Dic {
namespace Protocol {
// response
template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response);
template <> std::optional<document_t> ToResponseJson<FilesGetResponse>(const FilesGetResponse &response);
template <> std::optional<document_t> ToResponseJson<TokenHeartCheckResponse>(const TokenHeartCheckResponse &response);
template <> std::optional<document_t> ToResponseJson<ProjectExplorerInfoUpdateResponse>(
    const ProjectExplorerInfoUpdateResponse &response);
template <> std::optional<document_t> ToResponseJson<ProjectExplorerInfoGetResponse>(
    const ProjectExplorerInfoGetResponse &response);
template <> std::optional<document_t> ToResponseJson<ProjectExplorerInfoDeleteResponse>(
    const ProjectExplorerInfoDeleteResponse &response);
template <> std::optional<document_t> ToResponseJson<ProjectExplorerInfoClearResponse>(
    const ProjectExplorerInfoClearResponse &response);
template <>
std::optional<document_t> ToResponseJson<ProjectCheckValidResponse>(const ProjectCheckValidResponse &response);
template <> std::optional<document_t> ToResponseJson<BaselineSettingResponse>(const BaselineSettingResponse &response);
template <> std::optional<document_t> ToResponseJson<BaselineCancelResponse>(const BaselineCancelResponse &response);
// event
template <typename EVENT> std::optional<document_t> ToEventJson(const EVENT &event);
template <> std::optional<document_t> ToEventJson<ReadFileFailEvent>(const ReadFileFailEvent &event);
} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_GLOBAL_PROTOCOL_UTIL_H
