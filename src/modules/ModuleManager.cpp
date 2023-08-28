/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "GlobalModule.h"
#include "ModuleManager.h"
#include "TimelineModule.h"

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
    std::unique_ptr<GlobalModule> global = std::make_unique<GlobalModule>();
    std::unique_ptr<TimelineModule> timelineModule = std::make_unique<TimelineModule>();
    global->RegisterRequestHandlers();
    timelineModule->RegisterRequestHandlers();
    moduleMap.emplace(ModuleType::GLOBAL, std::move(global));
    moduleMap.emplace(ModuleType::TIMELINE, std::move(timelineModule));
}

void ModuleManager::UnRegister()
{
    std::unique_lock<std::mutex> lock(mutex);
    moduleMap.clear();
}

bool ModuleManager::SetGlobalConfig(const GlobalConfig &config)
{
    if (moduleMap.count(ModuleType::GLOBAL) == 0) {
        return false;
    }
    ((GlobalModule &)(*moduleMap.at(ModuleType::GLOBAL).get())).Config(config);
    return true;
}

const std::optional<GlobalConfig> ModuleManager::GetGlobalConfig()
{
    if (moduleMap.count(ModuleType::GLOBAL) == 0) {
        return std::nullopt;
    }
    return ((GlobalModule &)(*moduleMap.at(ModuleType::GLOBAL).get())).GetConfig();
}

void ModuleManager::OnDispatchModuleRequest(std::unique_ptr<Request> request)
{
    auto moduleName = request->moduleName;
    if (moduleMap.count(moduleName) == 0) {
        ServerLog::Error("Failed to dispatch to module, module = ", ENUM_TO_STR(moduleName).value(),
            ", token = ", StringUtil::AnonymousString(request->token), ", command = ", request->command);
        return;
    }
    ServerLog::Info("Dispatch to module, module = ", ENUM_TO_STR(moduleName).value(),
        ", token = ", StringUtil::AnonymousString(request->token), ", command = ", request->command,
        ", request id = ", request->id);
    moduleMap.at(moduleName)->OnRequest(std::move(request));
}
} // end of namespace Module
} // end of namespace Dic