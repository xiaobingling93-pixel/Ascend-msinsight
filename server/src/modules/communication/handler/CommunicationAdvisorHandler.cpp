/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "CommunicationAdvisorHandler.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "WsSessionManager.h"
#include "WsSender.h"
#include "CommunicationAdvisor.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic;
using namespace Dic::Server;

bool CommunicationAdvisorHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request =
        dynamic_cast<Protocol::CommunicationAdvisorRequest &>(*requestPtr);
    std::unique_ptr<Protocol::CommunicationAdvisorResponse> responsePtr =
        std::make_unique<Protocol::CommunicationAdvisorResponse>();
    CommunicationAdvisorResponse &response = *responsePtr;
    std::string errMsg;
    if (!request.params.CheckParams(errMsg)) {
        SendResponse(std::move(responsePtr), false, errMsg);
        return false;
    }
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    CommunicationAdvisor advisor;
    advisor.Register();
    advisor.GenerateAdvisor(response.body.items);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Communication
} // Module
} // Dic