/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "SourceModule.h"
#include "SourceRequestHandler.h"
#include "QueryCodeFileHandler.h"
#include "QueryApiLineHandler.h"
#include "QueryApiInstructionsHandler.h"

namespace Dic {
namespace Module {
using namespace Dic::Module::Source;
    SourceModule::SourceModule() : BaseModule()
{
    moduleName = ModuleType::SOURCE;
}

    SourceModule::~SourceModule()
{
    requestHandlerMap.clear();
}

void SourceModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_SOURCE_CODE_FILE, std::make_unique<QueryCodeFileHandler>());
    requestHandlerMap.emplace(REQ_RES_SOURCE_API_LINE, std::make_unique<QueryApiLineHandler>());
    requestHandlerMap.emplace(REQ_RES_SOURCE_API_INSTRUCTIONS, std::make_unique<QueryApiInstructionsHandler>());
}

void SourceModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
};
} // end of namespace Module
} // end of namespace Dic