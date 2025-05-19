 /*
  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
  */

#include "QueryAffinityOptimizerAdvice.h"
#include "QueryAffinityAPIAdvice.h"
#include "QueryAiCpuOpAdviceHandler.h"
#include "QueryAclnnOpAdvisorHandler.h"
#include "QueryFusedOpAdviceHandler.h"
#include "QueryOperatorDispatchHandler.h"
#include "ProtocolDefs.h"
#include "AdvisorModule.h"

namespace Dic::Module {
using namespace Dic::Module::Advisor;
AdvisorModule::AdvisorModule()
{
    moduleName = MODULE_ADVISOR;
}
AdvisorModule::~AdvisorModule()
{
    requestHandlerMap.clear();
}

void AdvisorModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_ADVISOR_AFFINITY_OPTIMIZER, std::make_unique<QueryAffinityOptimizerAdvice>());
    requestHandlerMap.emplace(REQ_RES_ADVISOR_AFFINITY_API, std::make_unique<QueryAffinityAPIAdvice>());
    requestHandlerMap.emplace(REQ_RES_ADVISOR_AICPU_OPERATORS, std::make_unique<QueryAiCpuOpAdviceHandler>());
    requestHandlerMap.emplace(REQ_RES_ADVISOR_ACLNN_OPERATORS, std::make_unique<QueryAclnnOpAdvisorHandler>());
    requestHandlerMap.emplace(REQ_RES_ADVISOR_OPERATORS_FUSION, std::make_unique<QueryFusedOpAdviceHandler>());
    requestHandlerMap.emplace(REQ_RES_ADVISOR_OPERATOR_DISPATCH, std::make_unique<QueryOperatorDispatchHandler>());
}

void AdvisorModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
}
}