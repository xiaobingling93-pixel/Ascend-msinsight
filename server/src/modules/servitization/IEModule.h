/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_IEMODULE_H
#define PROFILER_SERVER_IEMODULE_H
#include "BaseModule.h"
namespace Dic::Module {
class IEModule : public BaseModule {
public:
    IEModule();
    ~IEModule() override;
    void RegisterRequestHandlers() override;
    void OnRequest(std::unique_ptr<Protocol::Request> request) override;
};
}  // namespace Dic::Module
#endif  // PROFILER_SERVER_IEMODULE_H
