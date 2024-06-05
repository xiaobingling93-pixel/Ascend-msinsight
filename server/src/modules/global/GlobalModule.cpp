/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "GlobalModule.h"
#include "ServerLog.h"
#include "TokenCreateHandler.h"
#include "TokenDestroyHandler.h"
#include "TokenCheckHandler.h"
#include "TokenHeartCheckHandler.h"
#include "FilesGetHandler.h"
#include "UpdateProjectExplorerInfoHandler.h"
#include "GetProjectExplorerInfoHandler.h"
#include "DeleteProjectExplorerInfoHandler.h"
#include "CheckProjectConflictHandler.h"

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
    requestHandlerMap.emplace(REQ_RES_TOKEN_CREATE, std::make_unique<TokenCreateHandler>());
    requestHandlerMap.emplace(REQ_RES_TOKEN_DESTROY, std::make_unique<TokenDestroyHandler>());
    requestHandlerMap.emplace(REQ_RES_TOKEN_CHECK, std::make_unique<TokenCheckHandler>());
    requestHandlerMap.emplace(REQ_RES_TOKEN_HEART_CHECK, std::make_unique<TokenHeartCheckHandler>());
    requestHandlerMap.emplace(REQ_RES_FILES_GET, std::make_unique<FilesGetHandler>());
    requestHandlerMap.emplace(REQ_RES_PROJECT_EXPLORER_UPDATE,
                              std::make_unique<UpdateProjectExplorerInfoHandler>());
    requestHandlerMap.emplace(REQ_RES_PROJECT_EXPLORER_INFO_GET,
                              std::make_unique<GetProjectExplorerInfoHandler>());
    requestHandlerMap.emplace(REQ_RES_PROJECT_EXPLORER_INFO_DELETE,
                              std::make_unique<DeleteProjectExplorerInfoHandler>());
    requestHandlerMap.emplace(REQ_RES_PROJECT_CONFLICT_CHECK, std::make_unique<CheckProjectConflictHandler>());
}

void GlobalModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
};
} // end of namespace Module
} // end of namespace Dic