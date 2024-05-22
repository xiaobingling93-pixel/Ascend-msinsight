/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "JupyterModule.h"

namespace Dic {
namespace Module {
JupyterModule::JupyterModule() : BaseModule()
{
    moduleName = ModuleType::JUPYTER;
}

JupyterModule::~JupyterModule()
{
    requestHandlerMap.clear();
}

void JupyterModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
}

void JupyterModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
};
} // end of namespace Module
} // end of namespace Dic