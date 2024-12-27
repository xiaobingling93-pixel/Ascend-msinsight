/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "pch.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "ClusterService.h"
#include "IterationsHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic;
using namespace Dic::Server;
bool IterationsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    IterationsRequest &request = dynamic_cast<IterationsRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<IterationsOrRanksResponse> responsePtr = std::make_unique<IterationsOrRanksResponse>();
    IterationsOrRanksResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    ClusterService::QueryIterations(request, response);
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // Communication
} // Module
} // Dic