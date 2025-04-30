/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "CommunicationModule.h"
#include "CommunicationOperatorDetailsHandler.h"
#include "CommunicationOperatorListsHandler.h"
#include "BandwidthHandler.h"
#include "DistributionHandler.h"
#include "MatrixSortOpNamesHandler.h"
#include "IterationsHandler.h"
#include "DurationListHandler.h"
#include "OperatorNamesHandler.h"
#include "RanksHandler.h"
#include "GroupHandler.h"
#include "MatrixListHandler.h"
#include "CommunicationAdvisorHandler.h"
#include "ProtocolDefs.h"

namespace Dic {
namespace Module {
using namespace Communication;
CommunicationModule::CommunicationModule() : BaseModule()
{
    moduleName = MODULE_COMMUNICATION;
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
    requestHandlerMap.emplace(REQ_RES_COMMUNICATION_ITERATIONS,
                              std::make_unique<IterationsHandler>());
    requestHandlerMap.emplace(REQ_RES_COMMUNICATION_RANKS, std::make_unique<RanksHandler>());
    requestHandlerMap.emplace(REQ_RES_COMMUNICATION_OPERATORNAMES, std::make_unique<OperatorNamesHandler>());
    requestHandlerMap.emplace(REQ_RES_COMMUNICATION_SORT_OP, std::make_unique<MatrixSortOpNamesHandler>());
    requestHandlerMap.emplace(REQ_RES_COMMUNICATION_LIST, std::make_unique<DurationListHandler>());
    requestHandlerMap.emplace(REQ_RES_COMMUNICATION_MATRIX_GROUP, std::make_unique<GroupHandler>());
    requestHandlerMap.emplace(REQ_RES_COMMUNICATION_MATRIX_BANDWIDTH, std::make_unique<MatrixListHandler>());
    requestHandlerMap.emplace(REQ_RES_COMMUNICATION_OPERATOR_LISTS,
                              std::make_unique<CommunicationOperatorListsHandler>());
    requestHandlerMap.emplace(REQ_RES_COMMUNICATION_ADVISOR, std::make_unique<CommunicationAdvisorHandler>());
}

void CommunicationModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    BaseModule::OnRequest(std::move(request));
};
} // end of namespace Module
} // end of namespace Dic