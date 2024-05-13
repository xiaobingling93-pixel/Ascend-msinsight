 /*
  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
  */

#include "QueryAffinityOptimizerAdvice.h"
#include "AdvisorModule.h"

namespace Dic::Module {
using namespace Dic::Module::Advisor;
AdvisorModule::AdvisorModule()
{
    moduleName = ModuleType::ADVISOR;
}
AdvisorModule::~AdvisorModule()
{
    requestHandlerMap.clear();
}

void AdvisorModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_ADVISOR_AFFINITY_OPTIMIZER, std::make_unique<QueryAffinityOptimizerAdvice>());
}

void AdvisorModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
}
}