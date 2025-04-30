/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "CommunicationAdvisorHandler.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "WsSessionManager.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic;
using namespace Dic::Server;

bool CommunicationAdvisorHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    Protocol::CommunicationAdvisorRequest &request =
        dynamic_cast<Protocol::CommunicationAdvisorRequest &>(*requestPtr.get());
    std::unique_ptr<Protocol::CommunicationAdvisorResponse> responsePtr =
        std::make_unique<Protocol::CommunicationAdvisorResponse>();
    CommunicationAdvisorResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Communication
} // Module
} // Dic