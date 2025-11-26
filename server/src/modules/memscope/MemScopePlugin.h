/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#ifndef PROFILER_SERVER_MEM_SCOPE_PLUGIN_H
#define PROFILER_SERVER_MEM_SCOPE_PLUGIN_H
#include "BasePlugin.h"
#include "ProtocolDefs.h"
#include "MemScopeModule.h"
#include "MemScopeProtocol.h"
namespace Dic::Module::MemScope {
class MemScopePlugin : public Core::BasePlugin {
public:
    MemScopePlugin() : Core::BasePlugin(MODULE_MEM_SCOPE) {};
    std::unique_ptr<Module::BaseModule> GetModule() override
    {
        return std::make_unique<MemScopeModule>();
    }
    std::unique_ptr<Module::ProtocolUtil> GetProtocolUtil() override
    {
        return std::make_unique<MemScopeProtocolUtil>();
    }
};
}  // namespace Dic::Module::MemScope
#endif  // PROFILER_SERVER_MEM_SCOPE_PLUGIN_H
