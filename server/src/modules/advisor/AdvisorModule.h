 /*
  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
  */

#ifndef PROFILER_SERVER_ADVISORMODULE_H
#define PROFILER_SERVER_ADVISORMODULE_H

#include "BaseModule.h"
namespace Dic::Module {
class AdvisorModule : public BaseModule {
public:
    AdvisorModule();
    ~AdvisorModule()override;

    void RegisterRequestHandlers() override;
    void OnRequest(std::unique_ptr<Protocol::Request> request) override;
};
}

#endif // PROFILER_SERVER_ADVISORMODULE_H
