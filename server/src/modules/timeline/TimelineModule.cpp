//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
//
#include "pch.h"
#include "ProtocolDefs.h"
#include "QueryThreadTracesHandler.h"
#include "QueryThreadsHandler.h"
#include "QueryThreadDetailHandler.h"
#include "QuerySystemViewHandler.h"
#include "ResetWindowHandler.h"
#include "ImportActionHandler.h"
#include "SearchCountHandler.h"
#include "SearchSliceHandler.h"
#include "RemoteDeleteHandler.h"
#include "QueryFlowCategoryListHandler.h"
#include "QueryFlowCategoryEventsHandler.h"
#include "QueryUnitCounterHandler.h"
#include "QueryThreadTracesSummaryHandler.h"
#include "QueryKernelDetailHandler.h"
#include "QueryOneKernelHandler.h"
#include "QueryThreadsSameOperatorHandler.h"
#include "QueryFlowsBySliceInfoHandler.h"
#include "SearchAllSlicesHandler.h"
#include "QueryEventsViewHandler.h"
#include "QueryCommunicationKernelHandler.h"
#include "QuerySystemViewOverallHandler.h"
#include "QueryOverallMoreDetailsHandler.h"
#include "ParseCardsHandler.h"
#include "RenderEngine.h"
#include "DataEngine.h"
#include "RepositoryFactory.h"
#include "TimelineModule.h"

namespace Dic {
namespace Module {
using namespace Dic::Module::Timeline;
TimelineModule::TimelineModule() : BaseModule()
{
    moduleName = MODULE_TIMELINE;
}

TimelineModule::~TimelineModule()
{
    requestHandlerMap.clear();
}

void TimelineModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    auto respotoryFactory = RepositoryFactory::Instance();
    auto dataEngine = DataEngine::Instance();
    dataEngine->SetRepositoryFactory(respotoryFactory);
    auto renderEngine = RenderEngine::Instance();
    renderEngine->SetDataEngineInterface(dataEngine);

    auto queryThreadTracesHandler = std::make_unique<QueryThreadTracesHandler>();
    queryThreadTracesHandler->SetRenderEngine(renderEngine);
    requestHandlerMap.emplace(REQ_RES_UNIT_THREAD_TRACES, std::move(queryThreadTracesHandler));
    requestHandlerMap.emplace(REQ_RES_UNIT_THREAD_TRACES_SUMMARY, std::make_unique<QueryThreadTracesSummaryHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_THREADS, std::make_unique<QueryThreadsHandler>());

    auto queryThreadDetailHandler = std::make_unique<QueryThreadDetailHandler>();
    queryThreadDetailHandler->SetRenderEngine(renderEngine);
    requestHandlerMap.emplace(REQ_RES_UNIT_THREAD_DETAIL, std::move(queryThreadDetailHandler));

    requestHandlerMap.emplace(REQ_RES_UNIT_FLOWS, std::make_unique<QueryFlowsBySliceInfoHandler>());
    requestHandlerMap.emplace(REQ_RES_RESET_WINDOW, std::make_unique<ResetWindowHandler>());
    requestHandlerMap.emplace(REQ_RES_IMPORT_ACTION, std::make_unique<ImportActionHandler>());
    requestHandlerMap.emplace(REQ_RES_PARSE_CARDS, std::make_unique<ParseCardsHandler>());
    requestHandlerMap.emplace(REQ_RES_SEARCH_COUNT, std::make_unique<SearchCountHandler>());
    requestHandlerMap.emplace(REQ_RES_SEARCH_SLICE, std::make_unique<SearchSliceHandler>());
    requestHandlerMap.emplace(REQ_RES_REMOTE_DELETE, std::make_unique<RemoteDeleteHandler>());
    requestHandlerMap.emplace(REQ_RES_FLOW_CATEGORY_LIST, std::make_unique<QueryFlowCategoryListHandler>());

    auto queryFlowCategoryEventsHandler = std::make_unique<QueryFlowCategoryEventsHandler>();
    queryFlowCategoryEventsHandler->SetRenderEngine(renderEngine);
    requestHandlerMap.emplace(REQ_RES_FLOW_CATEGORY_EVENTS, std::move(queryFlowCategoryEventsHandler));

    requestHandlerMap.emplace(REQ_RES_UNIT_COUNTER, std::make_unique<QueryUnitCounterHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_SYSTEM_VIEW, std::make_unique<QuerySystemViewHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_KERNEL_DETAILS, std::make_unique<QueryKernelDetailHandler>());
    requestHandlerMap.emplace(REQ_RES_ONE_KERNEL_DETAILS, std::make_unique<QueryOneKernelHandler>());
    requestHandlerMap.emplace(REQ_RES_COMMUNICATION_KERNEL_DETAIL, std::make_unique<QueryCommunicationKernelHandler>());
    requestHandlerMap.emplace(REQ_RES_SAME_OPERATORS_DURATION, std::make_unique<QueryThreadsSameOperatorHandler>());
    requestHandlerMap.emplace(REQ_RES_SEARCH_ALL_SLICES, std::make_unique<SearchAllSlicesHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_EVENTS_VIEW, std::make_unique<QueryEventsViewHandler>());
    requestHandlerMap.emplace(REQ_RES_SYSTEM_VIEW_OVERALL, std::make_unique<QuerySystemViewOverallHandler>());
    requestHandlerMap.emplace(REQ_RES_SYSTEM_VIEW_OVERALL_MORE_DETAILS,
                              std::make_unique<QueryOverallMoreDetailsHandler>());
}

void TimelineModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
};
} // end of namespace Module
} // end of namespace Dic