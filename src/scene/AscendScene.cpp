//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
//

#include "AscendScene.h"
#include "ServerLog.h"
#include "QueryImportDataHandler.h"
#include "QueryThreadTracesHandler.h"
#include "QueryThreadsHandler.h"
#include "QueryThreadDetailHandler.h"
#include "QueryFlowNameHandler.h"
#include "QueryFlowHandler.h"
#include "ResetWindowHandler.h"
#include "QueryChartHandler.h"

namespace Dic {
namespace Scene {
using namespace Dic::Server;
AscendScene::AscendScene() : BaseScene()
{
    sceneType = SceneType::ASCEND;
}

AscendScene::~AscendScene()
{
    requestHandlerMap.clear();
}

void AscendScene::Config(const AscendConfig &cfg)
{
    config.maxSessionCount = cfg.maxSessionCount;
}

const AscendConfig &AscendScene::GetConfig() const
{
    return config;
}

void AscendScene::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_IMPORT_ACTION, std::make_unique<QueryImportDataHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_THREAD_TRACES, std::make_unique<QueryThreadTracesHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_THREADS, std::make_unique<QueryThreadsHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_THREAD_DETAIL, std::make_unique<QueryThreadDetailHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_FLOW_NAME, std::make_unique<QueryFlowNameHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_FLOW, std::make_unique<QueryFlowHandler>());
    requestHandlerMap.emplace(REQ_RES_RESET_WINDOW, std::make_unique<ResetWindowHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_CHART, std::make_unique<QueryChartHandler>());
}

void AscendScene::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseScene::OnRequest(std::move(request));
};
} // end of namespace Scene
} // end of namespace Dic