/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
#ifndef PROFILER_SERVER_QUERY_MEM_SCOPE_EVENT_HANDLER_H
#define PROFILER_SERVER_QUERY_MEM_SCOPE_EVENT_HANDLER_H

#include "MemScopeRequestHandler.h"

namespace Dic::Module::MemScope {
class QueryMemScopeEventHandler : public MemScopeRequestHandler {
public:
    QueryMemScopeEventHandler() { command = Protocol::REQ_RES_MEM_SCOPE_EVENTS; }
    ~QueryMemScopeEventHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}  // namespace Dic::Module::MemScope
#endif  // PROFILER_SERVER_QUERY_MEM_SCOPE_EVENT_HANDLER_H
