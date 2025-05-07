/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */
#include "GlobalModule.h"
#include "pch.h"
#include "HeartCheckHandler.h"
#include "FilesGetHandler.h"
#include "UpdateProjectExplorerInfoHandler.h"
#include "GetProjectExplorerInfoHandler.h"
#include "DeleteProjectExplorerInfoHandler.h"
#include "CheckProjectValidHandler.h"
#include "SetBaselineHandler.h"
#include "CancelBaselineHandler.h"
#include "ProtocolDefs.h"
#include "GetModuleConfigHandler.h"
#include "ClearProjectExplorerHandler.h"

using namespace Dic::Module;
using namespace Dic::Server;
using namespace Dic::Module::Global;
GlobalModule::GlobalModule() : BaseModule()
{
    moduleName = MODULE_GLOBAL;
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
    requestHandlerMap.emplace(REQ_RES_PROJECT_EXPLORER_CLEAR,
                              std::make_unique<ClearProjectExplorerHandler>());
    requestHandlerMap.emplace(REQ_RES_PROJECT_VALID_CHECK, std::make_unique<CheckProjectValidHandler>());
    requestHandlerMap.emplace(REQ_RES_PROJECT_SET_BASELINE, std::make_unique<SetBaselineHandler>());
    requestHandlerMap.emplace(REQ_RES_PROJECT_CANCEL_BASELINE, std::make_unique<CancelBaselineHandler>());
    requestHandlerMap.emplace(REQ_RES_GET_MODULE_CONFIG, std::make_unique<GetModuleConfigHandler>());
}

void GlobalModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
}