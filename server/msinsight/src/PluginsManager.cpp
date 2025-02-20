// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#include "ServerLog.h"
#include "./utils/FileUtil.h"
#include "PluginsManager.h"
namespace Dic::Core {
#ifdef _WIN32
    const static std::string EXT = ".dll";
#else
    #ifdef __APPLE__
        const static std::string EXT = ".dylib";
    #else
        const static std::string EXT = ".so";
    #endif
#endif
PluginsManager &PluginsManager::Instance()
{
    static PluginsManager instance;
    return instance;
}
bool PluginsManager::RegisterPlugin(std::unique_ptr<BasePlugin> plugin)
{
    if (pluginsMap_.count(plugin->GetPluginName()) != 0) {
        return false;
    }
    Server::ServerLog::Info("Load plugin:", plugin->GetPluginName());
    pluginsMap_.emplace(plugin->GetPluginName(), std::move(plugin));
    return true;
}
void PluginsManager::LoadPlugins()
{
    auto pluginsDir = fs::u8path(FileUtil::SplicePath(FileUtil::GetCurrPath(), "plugins"));
    if (!fs::exists(pluginsDir)) {
        return;
    }
    for (auto &dir : fs::directory_iterator(pluginsDir)) {
        if (!is_directory(dir)) {
            continue;
        }
        for (auto &file : fs::directory_iterator(dir)) {
            if (!is_directory(file) && file.path().extension().string() == EXT
                && FileUtil::CheckFilePath(file.path().string())) {
#ifdef _WIN32
                LoadLibraryA(file.path().string().c_str());
#else
                Server::ServerLog::Info("Load plugin so:", file.path().string());
                dlopen(file.path().string().c_str(), RTLD_LAZY);
#endif
            }
        }
    }
}
std::map<std::string, std::unique_ptr<BasePlugin>>& PluginsManager::GetAllPlugins()
{
    return pluginsMap_;
}
PluginRegister::PluginRegister(std::unique_ptr<BasePlugin> plugin)
{
    PluginsManager::Instance().RegisterPlugin(std::move(plugin));
}
}

