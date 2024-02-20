/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SOURCE_MODULE_H
#define PROFILER_SERVER_SOURCE_MODULE_H

#include "BaseModule.h"

namespace Dic {
namespace Module {
class SourceModule : public BaseModule {
public:
    SourceModule();
    ~SourceModule() override;

    void RegisterRequestHandlers() override;
    void OnRequest(std::unique_ptr<Protocol::Request> request) override;
};
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_SOURCE_MODULE_H
