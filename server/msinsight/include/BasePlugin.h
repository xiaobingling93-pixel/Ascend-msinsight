// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#ifndef MSINSIGHT_BASEPLUGIN_H
#define MSINSIGHT_BASEPLUGIN_H
#include "BaseModule.h"
#include "ProtocolUtil.h"
#include "ApiHandler.h"

namespace Dic::Core {
class BasePlugin {
public:
    explicit BasePlugin(std::string pluginName) : pluginName_(pluginName) {};
    virtual ~BasePlugin() = default;
    std::string GetPluginName() { return pluginName_; }
    virtual std::unique_ptr<Module::BaseModule> GetModule() { return nullptr; };
    virtual std::unique_ptr<Module::ProtocolUtil> GetProtocolUtil() { return nullptr; };
    virtual std::map<std::string, std::shared_ptr<ApiHandler>> GetAllHandlers() { return {}; };
    virtual std::vector<std::string> GetModuleConfig() { return {}; };
    virtual uint8_t GetOrder() { return UINT8_MAX; };
protected:
    std::string pluginName_;
};
struct PluginRegister {
    explicit PluginRegister(std::unique_ptr<BasePlugin> plugin);
};
}
#endif // MSINSIGHT_BASEPLUGIN_H
