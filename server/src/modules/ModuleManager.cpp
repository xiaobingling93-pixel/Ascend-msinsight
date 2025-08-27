/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "PluginsManager.h"
#include "ModuleManager.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
ModuleManager &ModuleManager::Instance()
{
    static ModuleManager instance;
    return instance;
}

ModuleManager::ModuleManager()
{
    Register();
}

ModuleManager::~ModuleManager()
{
    UnRegister();
}

void ModuleManager::Register()
{
    std::unique_lock<std::mutex> lock(mutex);
    moduleMap.clear();
    auto& manager = Core::PluginsManager::Instance();
    for (const auto &[moduleName, plugin]: manager.GetAllPlugins()) {
        auto module = plugin->GetModule();
        if (module != nullptr) {
            module->RegisterRequestHandlers();
            moduleMap.emplace(moduleName, std::move(module));
        }
    }
}

void ModuleManager::UnRegister()
{
    std::unique_lock<std::mutex> lock(mutex);
    moduleMap.clear();
}

void ModuleManager::OnDispatchModuleRequest(std::unique_ptr<Request> request)
{
    auto moduleName = request->moduleName;
    if (moduleMap.count(moduleName) == 0) {
        ServerLog::Error("Failed to dispatch to module");
        return;
    }
    if (!WsSessionManager::Instance().CheckSession()) {
        ServerLog::Error("Invalid session found when dispatch");
        return;
    }
    moduleMap.at(moduleName)->OnRequest(std::move(request));
}
} // end of namespace Module
} // end of namespace Dic