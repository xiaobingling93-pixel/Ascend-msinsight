/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_IEPLUGIN_H
#define PROFILER_SERVER_IEPLUGIN_H
#include "BasePlugin.h"
#include "IEModule.h"
#include "IEProtocol.h"
namespace Dic::Module::IE {
    class IEPlugin : public Core::BasePlugin {
    public:
        IEPlugin() : Core::BasePlugin(MODULE_IE) {};
        std::unique_ptr<Module::BaseModule> GetModule() override
        {
            return std::make_unique<IEModule>();
        }
        std::unique_ptr<Module::ProtocolUtil> GetProtocolUtil() override
        {
            return std::make_unique<IEProtocol>();
        }
    };
}
#endif // PROFILER_SERVER_IEPLUGIN_H
