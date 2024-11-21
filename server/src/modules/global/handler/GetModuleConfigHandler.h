/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_GETMODULECONFIGHANDLER_H
#define PROFILER_SERVER_GETMODULECONFIGHANDLER_H

#include "GlobalHandler.h"

namespace Dic {
namespace Module {
namespace Global {
class GetModuleConfigHandler : public GlobalHandler {
public:
    GetModuleConfigHandler()
    {
        command = REQ_RES_GET_MODULE_CONFIG;
    }
    ~GetModuleConfigHandler() override = default;
    bool HandleRequest(std::unique_ptr<Request> requestPtr) override;
};
} // Global
} // Module
} // Dic

#endif // PROFILER_SERVER_GETMODULECONFIGHANDLER_H
