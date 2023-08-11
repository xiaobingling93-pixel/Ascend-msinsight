/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "WsSessionManager.h"
#include "HdcListDeviceHandler.h"

namespace Dic {
namespace Scene {
using namespace Dic::Server;
void HdcListDeviceHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    HdcDeviceListRequest &request = dynamic_cast<HdcDeviceListRequest &>(*requestPtr.get());
    std::string token = request.token;
    ServerLog::Info("Hdc list device start, token = ", StringUtil::AnonymousString(token));
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
            ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<HdcDeviceListResponse> responsePtr = std::make_unique<HdcDeviceListResponse>();
    HdcDeviceListResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    // do sth.

    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // end of namespace Scene
} // end of namespace Dic