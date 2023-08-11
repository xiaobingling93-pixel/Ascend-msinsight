/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "GlobalScene.h"
#include "HarmonyScene.h"
#include "SceneManager.h"

namespace Dic {
namespace Scene {
using namespace Dic::Server;
SceneManager &SceneManager::Instance()
{
    static SceneManager instance;
    return instance;
}

SceneManager::SceneManager()
{
    Register();
}

SceneManager::~SceneManager()
{
    UnRegister();
}

void SceneManager::Register()
{
    std::unique_lock<std::mutex> lock(mutex);
    sceneMap.clear();
    std::unique_ptr<GlobalScene> globalScene = std::make_unique<GlobalScene>();
    std::unique_ptr<HarmonyScene> harmonyScene = std::make_unique<HarmonyScene>();
    harmonyScene->RegisterRequestHandlers();
    globalScene->RegisterRequestHandlers();
    sceneMap.emplace(SceneType::GLOBAL, std::move(globalScene));
    sceneMap.emplace(SceneType::HARMONY, std::move(harmonyScene));
}

void SceneManager::UnRegister()
{
    std::unique_lock<std::mutex> lock(mutex);
    sceneMap.clear();
}

bool SceneManager::SetGlobalConfig(const GlobalConfig &config)
{
    if (sceneMap.count(SceneType::GLOBAL) == 0) {
        return false;
    }
    ((GlobalScene &)(*sceneMap.at(SceneType::GLOBAL).get())).Config(config);
    return true;
}

bool SceneManager::SetHarmonyConfig(const HarmonyConfig &config)
{
    if (sceneMap.count(SceneType::HARMONY) == 0) {
        return false;
    }
    ((HarmonyScene &)(*sceneMap.at(SceneType::HARMONY).get())).Config(config);
    return true;
}

const std::optional<GlobalConfig> SceneManager::GetGlobalConfig()
{
    if (sceneMap.count(SceneType::GLOBAL) == 0) {
        return std::nullopt;
    }
    return ((GlobalScene &)(*sceneMap.at(SceneType::GLOBAL).get())).GetConfig();
}

const std::optional<HarmonyConfig> SceneManager::GetHarmonyConfig()
{
    if (sceneMap.count(SceneType::HARMONY) == 0) {
        return std::nullopt;
    }
    return ((HarmonyScene &)(*sceneMap.at(SceneType::HARMONY).get())).GetConfig();
}

void SceneManager::OnDispatchSceneRequest(std::unique_ptr<Request> request)
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
} // end of namespace Scene
} // end of namespace Dic