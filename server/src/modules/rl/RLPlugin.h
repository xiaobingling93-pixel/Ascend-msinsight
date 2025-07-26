/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_RLPLUGIN_H
#define PROFILER_SERVER_RLPLUGIN_H

#include "BasePlugin.h"
#include "ProtocolDefs.h"
#include "RLModule.h"
#include "RLProtocol.h"
namespace Dic::Module::RL {
    class RLPlugin : public Core::BasePlugin {
    public:
        RLPlugin() : Core::BasePlugin(MODULE_RL) {};
        std::unique_ptr<Module::BaseModule> GetModule() override
        {
            return std::make_unique<RLModule>();
        }
        std::unique_ptr<Module::ProtocolUtil> GetProtocolUtil() override
        {
            return std::make_unique<RLProtocol>();
        }
    };
}
#endif // PROFILER_SERVER_RLPLUGIN_H
