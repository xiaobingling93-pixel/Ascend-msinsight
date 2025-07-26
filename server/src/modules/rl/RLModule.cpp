/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "RLModule.h"
#include "ProtocolDefs.h"
#include "RLPipelineHandler.h"

namespace Dic::Module {
using namespace Dic::Module::RL;

RLModule::RLModule() : BaseModule()
{
    moduleName = MODULE_RL;
}

RLModule::~RLModule()
{
    requestHandlerMap.clear();
}

void RLModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_REQ_RL_PIPELINE, std::make_unique<RLPipelineHandler>());
}

void RLModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
}
}
