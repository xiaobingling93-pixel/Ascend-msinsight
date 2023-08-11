/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "TokenCreateHandler.h"
#include "TokenDestroyHandler.h"
#include "TokenCheckHandler.h"
#include "ConfigGetHandler.h"
#include "ConfigSetHandler.h"
#include "GlobalScene.h"

namespace Dic {
namespace Scene {
using namespace Dic::Server;
GlobalScene::GlobalScene() : BaseScene()
{
    sceneType = SceneType::GLOBAL;
}

GlobalScene::~GlobalScene()
{
    requestHandlerMap.clear();
}

void GlobalScene::Config(const GlobalConfig &cfg)
{
    config.maxSessionCount = cfg.maxSessionCount;
}

const GlobalConfig &GlobalScene::GetConfig() const
{
    return config;
}

void GlobalScene::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_TOKEN_CREATE, std::make_unique<TokenCreateHandler>());
    requestHandlerMap.emplace(REQ_RES_TOKEN_DESTROY, std::make_unique<TokenDestroyHandler>());
    requestHandlerMap.emplace(REQ_RES_TOKEN_CHECK, std::make_unique<TokenCheckHandler>());
    requestHandlerMap.emplace(REQ_RES_CONFIG_GET, std::make_unique<ConfigGetHandler>());
    requestHandlerMap.emplace(REQ_RES_CONFIG_SET, std::make_unique<ConfigSetHandler>());
}

void GlobalScene::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseScene::OnRequest(std::move(request));
};
} // end of namespace Scene
} // end of namespace Dic