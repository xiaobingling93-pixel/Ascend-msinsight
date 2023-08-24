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
    sceneMap.clear();
    std::unique_ptr<GlobalModule> globalScene = std::make_unique<GlobalModule>();
    std::unique_ptr<TimelineModule> ascendScene = std::make_unique<TimelineModule>();
    globalScene->RegisterRequestHandlers();
    ascendScene->RegisterRequestHandlers();
    sceneMap.emplace(ModuleType::GLOBAL, std::move(globalScene));
    sceneMap.emplace(ModuleType::TIMELINE, std::move(ascendScene));
}

void ModuleManager::UnRegister()
{
    std::unique_lock<std::mutex> lock(mutex);
    sceneMap.clear();
}

bool ModuleManager::SetGlobalConfig(const GlobalConfig &config)
{
    if (sceneMap.count(ModuleType::GLOBAL) == 0) {
        return false;
    }
    ((GlobalModule &)(*sceneMap.at(ModuleType::GLOBAL).get())).Config(config);
    return true;
}

bool ModuleManager::SetAscendConfig(const TimelineConfig &config) {
    if (sceneMap.count(ModuleType::TIMELINE) == 0) {
        return false;
    }
    ((TimelineModule &)(*sceneMap.at(ModuleType::TIMELINE).get())).Config(config);
    return true;
}

const std::optional<GlobalConfig> ModuleManager::GetGlobalConfig()
{
    if (sceneMap.count(ModuleType::GLOBAL) == 0) {
        return std::nullopt;
    }
    return ((GlobalModule &)(*sceneMap.at(ModuleType::GLOBAL).get())).GetConfig();
}

const std::optional<TimelineConfig> ModuleManager::GetAscendConfig() {
    if (sceneMap.count(ModuleType::TIMELINE) == 0) {
        return std::nullopt;
    }
    return ((TimelineModule &)(*sceneMap.at(ModuleType::TIMELINE).get())).GetConfig();
}

void ModuleManager::OnDispatchSceneRequest(std::unique_ptr<Request> request)
{
    if (sceneMap.count(request->scene) == 0) {
        ServerLog::Error("Failed to dispatch to scene, scene = ", ENUM_TO_STR(request->scene).value(),
            ", token = ", StringUtil::AnonymousString(request->token), ", command = ", request->command);
        return;
    }
    int callbackId = request->resultCallbackId.has_value() ? request->resultCallbackId.value() : -1;
    ServerLog::Info("Dispatch to scene, scene = ", ENUM_TO_STR(request->scene).value(),
        ", token = ", StringUtil::AnonymousString(request->token), ", command = ", request->command,
        ", request id = ", request->id, ", callback id = ", callbackId);
    sceneMap.at(request->scene)->OnRequest(std::move(request));
}
} // end of namespace Module
} // end of namespace Dic