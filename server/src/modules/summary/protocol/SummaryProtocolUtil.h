/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_SUMMARY_PROTOCOLUTIL_H
#define PROFILER_SERVER_SUMMARY_PROTOCOLUTIL_H

#include <optional>
#include "GlobalDefs.h"

namespace Dic {
namespace Protocol {
    // response
    template<typename RESPONSE>std::optional<json_t> ToResponseJson(const RESPONSE &response);

} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_SUMMARY_PROTOCOLUTIL_H
