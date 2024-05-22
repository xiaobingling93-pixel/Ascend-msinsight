/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_JUPYTERPROTOCOL_H
#define PROFILER_SERVER_JUPYTERPROTOCOL_H

#include "ProtocolUtil.h"

namespace Dic {
namespace Protocol {
class JupyterProtocol : public ProtocolUtil {
public:
    JupyterProtocol() = default;
    ~JupyterProtocol() override = default;
private:
    void RegisterJsonToRequestFuncs() override;
    void RegisterResponseToJsonFuncs() override;
    void RegisterEventToJsonFuncs() override;
    static std::optional<document_t> ToParseJupyterCompletedEventJson(const Event &event);
};
}
}

#endif // PROFILER_SERVER_JUPYTERPROTOCOL_H
