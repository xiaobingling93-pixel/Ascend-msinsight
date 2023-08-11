/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "HarmonyScene.h"
#include "HdcListDeviceHandler.h"

namespace Dic {
namespace Scene {
HarmonyScene::HarmonyScene() : BaseScene()
{
    sceneType = SceneType::HARMONY;
}

HarmonyScene::~HarmonyScene()
{
    requestHandlerMap.clear();
}

void HarmonyScene::Config(const HarmonyConfig &cfg)
{
    config = cfg;
}

const HarmonyConfig &HarmonyScene::GetConfig() const
{
    return config;
}

void HarmonyScene::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    RegisterHdcRequestHandlers();
    RegisterDfxRequestHandlers();
}

void HarmonyScene::RegisterHdcRequestHandlers()
{
    (void)requestHandlerMap.emplace(REQ_RES_HDC_DEVICE_LIST, std::make_unique<HdcListDeviceHandler>());
}

void HarmonyScene::RegisterDfxRequestHandlers()
{

}

void HarmonyScene::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseScene::OnRequest(std::move(request));
}
} // end of namespace Scene
} // end of namespace Dic
