/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#ifndef PROFILER_SERVER_MEM_SCOPE_MODULE_H
#define PROFILER_SERVER_MEM_SCOPE_MODULE_H

#include "BaseModule.h"

namespace Dic {
namespace Module {
class MemScopeModule : public BaseModule {
public:
    MemScopeModule();
    ~MemScopeModule() override;

    void RegisterRequestHandlers() override;
    void OnRequest(std::unique_ptr<Protocol::Request> request) override;
};
}  // end of namespace Module
}  // end of namespace Dic

#endif  // PROFILER_SERVER_MEM_SCOPE_MODULE_H
