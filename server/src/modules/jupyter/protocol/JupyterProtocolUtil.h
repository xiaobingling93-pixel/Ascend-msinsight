/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef PROFILER_SERVER_JUPYTERPROTOCOLUTIL_H
#define PROFILER_SERVER_JUPYTERPROTOCOLUTIL_H

#include <optional>
#include "GlobalDefs.h"
#include "JupyterProtocolEvent.h"

namespace Dic {
namespace Protocol {
    template <typename EVENT> std::optional<document_t> ToEventJson(const EVENT &event);
    template <>
    std::optional<document_t> ToEventJson<ParseJupyterCompletedEvent>(const ParseJupyterCompletedEvent &event);
}
}

#endif // PROFILER_SERVER_JUPYTERPROTOCOLUTIL_H
