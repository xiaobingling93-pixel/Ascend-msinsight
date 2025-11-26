/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#ifndef PROFILER_SERVER_MEM_SCOPE_PROTOCOL_H
#define PROFILER_SERVER_MEM_SCOPE_PROTOCOL_H
#include "ProtocolMessage.h"


namespace Dic::Protocol {
class MemScopeProtocolUtil : public ProtocolUtil {
public:
    MemScopeProtocolUtil() = default;
    ~MemScopeProtocolUtil() override = default;

private:
    void RegisterJsonToRequestFuncs() override;
    void RegisterResponseToJsonFuncs() override;
    void RegisterEventToJsonFuncs() override;
};
} // end of namespace Dic::Protocol
#endif  // PROFILER_SERVER_MEM_SCOPE_PROTOCOL_H
