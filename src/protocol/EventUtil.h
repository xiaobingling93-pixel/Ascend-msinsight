/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_PROTOCOL_EVENT_UTIL_H
#define DATA_INSIGHT_CORE_PROTOCOL_EVENT_UTIL_H

#include <optional>
#include "GlobalDefs.h"
#include "ProtocolBase.h"
#include "ProtocolEvent.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Protocol;
template <typename EVENT> std::optional<json_t> ToEventJson(const EVENT &event);

template <> std::optional<json_t> ToEventJson<ParseSuccessEvent>(const ParseSuccessEvent &event);
} // end of namespace Protocol
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_PROTOCOL_EVENT_UTIL_H
