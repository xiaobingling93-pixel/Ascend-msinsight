/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "TokenCreateHandler.h"
#include "TokenDestroyHandler.h"
#include "TokenCheckHandler.h"
#include "FilesGetHandler.h"
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
    requestHandlerMap.emplace(REQ_RES_TOKEN_CREATE, std::make_unique<TokenCreateHandler>());
    requestHandlerMap.emplace(REQ_RES_TOKEN_DESTROY, std::make_unique<TokenDestroyHandler>());
    requestHandlerMap.emplace(REQ_RES_TOKEN_CHECK, std::make_unique<TokenCheckHandler>());
    requestHandlerMap.emplace(REQ_RES_FILES_GET, std::make_unique<FilesGetHandler>());
}

void GlobalModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
};
} // end of namespace Module
} // end of namespace Dic