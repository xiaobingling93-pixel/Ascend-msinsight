/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "pch.h"
#include "CommunicationProtocolRequest.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "CommunicatorGroupQueryHandler.h"

namespace Dic::Module::Communication {
using namespace Dic::Server;
using namespace Dic::Protocol;
bool CommunicatorGroupQueryHandler::HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<Protocol::CommunicatorGroupRequest &>(*requestPtr);
    std::unique_ptr<Protocol::CommunicatorGroupResponse> responsePtr =
            std::make_unique<Protocol::CommunicatorGroupResponse>();
    CommunicatorGroupResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    SetResponseResult(response, true);
    auto database = Module::Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    if (!database->QueryCommunicationGroup(response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Communication CommunicationGroupQueryHandler Failed");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // end of namespace Communication
// end of namespace Module
// end of namespace Dic