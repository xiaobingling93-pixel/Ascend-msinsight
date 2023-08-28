/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "TokenCreateHandler.h"
#include "TokenDestroyHandler.h"
#include "TokenCheckHandler.h"
#include "ConfigGetHandler.h"
#include "ConfigSetHandler.h"
#include "GlobalModule.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
GlobalModule::GlobalModule() : BaseModule()
{
    moduleType = ModuleType::GLOBAL;
}

GlobalModule::~GlobalModule()
{
    requestHandlerMap.clear();
}

void GlobalModule::Config(const GlobalConfig &cfg)
{
    config.maxSessionCount = cfg.maxSessionCount;
}

const GlobalConfig &GlobalModule::GetConfig() const
{
    return config;
}

void GlobalModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_TOKEN_CREATE, std::make_unique<TokenCreateHandler>());
    requestHandlerMap.emplace(REQ_RES_TOKEN_DESTROY, std::make_unique<TokenDestroyHandler>());
    requestHandlerMap.emplace(REQ_RES_TOKEN_CHECK, std::make_unique<TokenCheckHandler>());
    requestHandlerMap.emplace(REQ_RES_CONFIG_GET, std::make_unique<ConfigGetHandler>());
    requestHandlerMap.emplace(REQ_RES_CONFIG_SET, std::make_unique<ConfigSetHandler>());
}

void GlobalModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
};
} // end of namespace Module
} // end of namespace Dic