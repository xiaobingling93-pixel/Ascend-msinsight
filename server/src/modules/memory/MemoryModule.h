/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORY_MODULE_H
#define PROFILER_SERVER_MEMORY_MODULE_H

#include "BaseModule.h"

namespace Dic {
namespace Module {
class MemoryModule : public BaseModule {
public:
    MemoryModule();
    ~MemoryModule() override;

    void RegisterRequestHandlers() override;
    void OnRequest(std::unique_ptr<Protocol::Request> request) override;
};
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_MEMORY_MODULE_H
