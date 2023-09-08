/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATION_MODULE_H
#define PROFILER_SERVER_COMMUNICATION_MODULE_H

#include "BaseModule.h"

namespace Dic {
namespace Module {
class CommunicationModule : public BaseModule {
public:
    CommunicationModule();
    ~CommunicationModule() override;

    void RegisterRequestHandlers() override;
    void OnRequest(std::unique_ptr<Protocol::Request> request) override;
};
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_COMMUNICATION_MODULE_H
