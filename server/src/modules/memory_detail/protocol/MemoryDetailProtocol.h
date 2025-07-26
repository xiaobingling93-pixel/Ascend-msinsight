/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#ifndef PROFILER_SERVER_MEMORY_DETAIL_PROTOCOL_H
#define PROFILER_SERVER_MEMORY_DETAIL_PROTOCOL_H
#include "ProtocolMessage.h"


namespace Dic::Protocol {
class MemoryDetailProtocolUtil : public ProtocolUtil {
public:
    MemoryDetailProtocolUtil() = default;
    ~MemoryDetailProtocolUtil() override = default;

private:
    void RegisterJsonToRequestFuncs() override;
    void RegisterResponseToJsonFuncs() override;
    void RegisterEventToJsonFuncs() override;
};
} // end of namespace Dic::Protocol
#endif  // PROFILER_SERVER_MEMORY_DETAIL_PROTOCOL_H
