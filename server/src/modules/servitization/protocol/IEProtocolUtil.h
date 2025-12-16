/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
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
