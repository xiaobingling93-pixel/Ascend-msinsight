/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "CommunicationModule.h"
#include "CommunicationOperatorDetailsHandler.h"
#include "CommunicatorGroupParserHandler.h"
#include "BandwidthHandler.h"
#include "DistributionHandler.h"

namespace Dic {
namespace Module {
using namespace Communication;
CommunicationModule::CommunicationModule() : BaseModule()
{
    moduleName = ModuleType::COMMUNICATION;
}

CommunicationModule::~CommunicationModule()
{
    requestHandlerMap.clear();
}

void CommunicationModule::RegisterRequestHandlers()
{
    requestHandlerMap.clear();
    requestHandlerMap.emplace(REQ_RES_COMMUNICATION_OPERATOR_DETAILS,
                              std::make_unique<CommunicationOperatorDetailsHandler>());
    requestHandlerMap.emplace(REQ_RES_COMMUNICATION_BANDWIDTH, std::make_unique<BandwidthHandler>());
    requestHandlerMap.emplace(REQ_RES_COMMUNICATION_DISTRIBUTION, std::make_unique<DistributionHandler>());
    requestHandlerMap.emplace(REQ_RES_COMMUNICATOR_PARSE, std::make_unique<CommunicatorGroupParserHandler>());
}

void CommunicationModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
};
} // end of namespace Module
} // end of namespace Dic