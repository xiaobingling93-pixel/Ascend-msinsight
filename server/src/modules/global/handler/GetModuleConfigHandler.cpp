/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "GetModuleConfigHandler.h"
#include "JsonUtil.h"
#include "PluginsManager.h"

namespace Dic::Module::Global {

bool GetModuleConfigHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    auto& manager = Core::PluginsManager::Instance();
    std::unique_ptr<ModuleConfigGetResponse> responsePtr = std::make_unique<ModuleConfigGetResponse>();
    std::vector<std::pair<uint8_t, std::vector<std::string>>> configWithOrder;
    for (const auto &[pluginName, plugin]: manager.GetAllPlugins()) {
        auto moduleConfig = plugin->GetModuleConfig();
        if (!moduleConfig.empty()) {
            configWithOrder.push_back(std::make_pair(plugin->GetOrder(), plugin->GetModuleConfig()));
        }
    }
    std::sort(configWithOrder.begin(), configWithOrder.end(), [](auto cmpA, auto cmpB) {
        return cmpA.first < cmpB.first ;
    });
    for (const auto &[order, configs]: configWithOrder) {
        responsePtr->configs.insert(responsePtr->configs.end(), configs.begin(), configs.end());
    }
    SetBaseResponse(*requestPtr, *responsePtr);
    SendResponse(std::move(responsePtr), true);
    return true;
}
}