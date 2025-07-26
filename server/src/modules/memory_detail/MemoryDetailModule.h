/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#ifndef PROFILER_SERVER_MEMORY_DETAIL_MODULE_H
#define PROFILER_SERVER_MEMORY_DETAIL_MODULE_H

#include "BaseModule.h"

namespace Dic {
namespace Module {
class MemoryDetailModule : public BaseModule {
public:
    MemoryDetailModule();
    ~MemoryDetailModule() override;

    void RegisterRequestHandlers() override;
    void OnRequest(std::unique_ptr<Protocol::Request> request) override;
};
}  // end of namespace Module
}  // end of namespace Dic

#endif  // PROFILER_SERVER_MEMORY_DETAIL_MODULE_H
