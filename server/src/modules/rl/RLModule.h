/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_RLMODULE_H
#define PROFILER_SERVER_RLMODULE_H

#include "BaseModule.h"

namespace Dic::Module {
class RLModule : public BaseModule {
public:
    RLModule();
    ~RLModule() override;

    void RegisterRequestHandlers() override;
    void OnRequest(std::unique_ptr<Protocol::Request> request) override;
};
}

#endif // PROFILER_SERVER_RLMODULE_H
