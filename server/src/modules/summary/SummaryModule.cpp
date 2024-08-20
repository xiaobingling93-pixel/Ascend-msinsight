/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "SummaryTopRankHandler.h"
#include "SummaryStatisticsHandler.h"
#include "QueryComputeDetailInfoHandler.h"
#include "StepHandler.h"
#include "StageHandler.h"
#include "StageAndBubbleTimeHandler.h"
#include "RankAndBubbleTimeHandler.h"
#include "QueryCommunicationDetailHandler.h"
#include "QueryParallelStrategyConfigHandler.h"
#include "SetParallelStrategyConfigHandler.h"
#include "SummaryModule.h"

namespace Dic {
namespace Module {
using namespace Dic::Module::Summary;
SummaryModule::SummaryModule() : BaseModule()
{
    moduleName = ModuleType::SUMMARY;
}

SummaryModule::~SummaryModule()
{
    requestHandlerMap.clear();
}

void SummaryModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_SUMMARY_QUERY_TOP_DATA, std::make_unique<SummaryTopRankHandler>());
    requestHandlerMap.emplace(REQ_RES_SUMMARY_STATISTIC, std::make_unique<SummaryStatisticsHandler>());
    requestHandlerMap.emplace(REQ_RES_COMPUTE_DETAIL, std::make_unique<QueryComputeDetailInfoHandler>());
    requestHandlerMap.emplace(REQ_RES_PIPELINE_GET_ALL_STEPS, std::make_unique<StepHandler>());
    requestHandlerMap.emplace(REQ_RES_PIPELINE_GET_ALL_STAGES, std::make_unique<StageHandler>());
    requestHandlerMap.emplace(REQ_RES_PIPELINE_STAGE_BUBBLE, std::make_unique<StageAndBubbleTimeHandler>());
    requestHandlerMap.emplace(REQ_RES_PIPELINE_RANK_BUBBLE, std::make_unique<RankAndBubbleTimeHandler>());
    requestHandlerMap.emplace(REQ_RES_COMMUNICATION_DETAIL, std::make_unique<QueryCommunicationDetailHandler>());
    requestHandlerMap.emplace(REQ_RES_SUMMARY_QUERY_PARALLEL_STRATEGY,
                              std::make_unique<QueryParallelStrategyConfigHandler>());
    requestHandlerMap.emplace(REQ_RES_SUMMARY_SET_PARALLEL_STRATEGY,
                              std::make_unique<SetParallelStrategyConfigHandler>());
}

void SummaryModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
};
} // end of namespace Module
} // end of namespace Dic