/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */
#include "pch.h"
#include "HeartCheckHandler.h"
#include "FilesGetHandler.h"
#include "UpdateProjectExplorerInfoHandler.h"
#include "GetProjectExplorerInfoHandler.h"
#include "DeleteProjectExplorerInfoHandler.h"
#include "CheckProjectValidHandler.h"
#include "GlobalModule.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Dic::Module::Global;
GlobalModule::GlobalModule() : BaseModule()
{
    moduleName = ModuleType::GLOBAL;
}

GlobalModule::~GlobalModule()
{
    requestHandlerMap.clear();
}

void GlobalModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_HEART_CHECK, std::make_unique<HeartCheckHandler>());
    requestHandlerMap.emplace(REQ_RES_FILES_GET, std::make_unique<FilesGetHandler>());
    requestHandlerMap.emplace(REQ_RES_PROJECT_EXPLORER_UPDATE,
                              std::make_unique<UpdateProjectExplorerInfoHandler>());
    requestHandlerMap.emplace(REQ_RES_PROJECT_EXPLORER_INFO_GET,
                              std::make_unique<GetProjectExplorerInfoHandler>());
    requestHandlerMap.emplace(REQ_RES_PROJECT_EXPLORER_INFO_DELETE,
                              std::make_unique<DeleteProjectExplorerInfoHandler>());
    requestHandlerMap.emplace(REQ_RES_PROJECT_VALID_CHECK, std::make_unique<CheckProjectValidHandler>());
}

void GlobalModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
};
} // end of namespace Module
} // end of namespace Dic