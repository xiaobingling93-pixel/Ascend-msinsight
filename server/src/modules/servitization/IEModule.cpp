/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "ProtocolDefs.h"
#include "IECurveHandler.h"
#include "IECurveTableDatailHandler.h"
#include "CurveGroupHandler.h"
#include "IEModule.h"
namespace Dic::Module {
using namespace Dic::Module::IE;
IEModule::IEModule() : BaseModule()
{
    moduleName = MODULE_IE;
}

IEModule::~IEModule()
{
    requestHandlerMap.clear();
}

void IEModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_IE_VIEW, std::move(std::make_unique<IECurveHandler>()));
    requestHandlerMap.emplace(REQ_RES_IE_TABLE_VIEW, std::move(std::make_unique<IECurveTableDatailHandler>()));
    requestHandlerMap.emplace(REQ_RES_IE_DATA_GROUP, std::move(std::make_unique<CurveGroupHandler>()));
}

void IEModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
}
}  // namespace Dic::Module