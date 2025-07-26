/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#ifndef PROFILER_SERVER_MEMORY_DETAIL_PLUGIN_H
#define PROFILER_SERVER_MEMORY_DETAIL_PLUGIN_H
#include "BasePlugin.h"
#include "ProtocolDefs.h"
#include "MemoryDetailModule.h"
#include "MemoryDetailProtocol.h"
namespace Dic::Module::MemoryDetail {
class MemoryDetailPlugin : public Core::BasePlugin {
public:
    MemoryDetailPlugin() : Core::BasePlugin(MODULE_LEAKS) {};
    std::unique_ptr<Module::BaseModule> GetModule() override
    {
        return std::make_unique<MemoryDetailModule>();
    }
    std::unique_ptr<Module::ProtocolUtil> GetProtocolUtil() override
    {
        return std::make_unique<MemoryDetailProtocolUtil>();
    }
};
}  // namespace Dic::Module::MemoryDetail
#endif  // PROFILER_SERVER_MEMORY_DETAIL_PLUGIN_H
