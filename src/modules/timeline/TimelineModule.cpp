//
//  * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
//

#include "TimelineModule.h"
#include "ServerLog.h"
#include "QueryThreadTracesHandler.h"
#include "QueryThreadsHandler.h"
#include "QueryThreadDetailHandler.h"
#include "QueryFlowNameHandler.h"
#include "QueryFlowHandler.h"
#include "ResetWindowHandler.h"
#include "QueryChartHandler.h"
#include "ImportActionHandler.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Dic::Module::Timeline;
TimelineModule::TimelineModule() : BaseModule()
{
    moduleName = ModuleType::TIMELINE;
}

TimelineModule::~TimelineModule()
{
    requestHandlerMap.clear();
}

void TimelineModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_UNIT_THREAD_TRACES, std::make_unique<QueryThreadTracesHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_THREADS, std::make_unique<QueryThreadsHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_THREAD_DETAIL, std::make_unique<QueryThreadDetailHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_FLOW_NAME, std::make_unique<QueryFlowNameHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_FLOW, std::make_unique<QueryFlowHandler>());
    requestHandlerMap.emplace(REQ_RES_RESET_WINDOW, std::make_unique<ResetWindowHandler>());
    requestHandlerMap.emplace(REQ_RES_UNIT_CHART, std::make_unique<QueryChartHandler>());
    requestHandlerMap.emplace(REQ_RES_IMPORT_ACTION, std::make_unique<ImportActionHandler>());
}

void TimelineModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
};
} // end of namespace Module
} // end of namespace Dic