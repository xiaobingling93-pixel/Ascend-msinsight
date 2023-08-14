//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#include "QueryThreadTracesHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"

namespace Dic {
namespace Scene {
using namespace Dic::Server;
void QueryThreadTracesHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) {
    UnitThreadTracesRequest &request = dynamic_cast<UnitThreadTracesRequest &>(*requestPtr.get());
    std::string token = request.token;
    ServerLog::Info("Hdc list device start, token = ", StringUtil::AnonymousString(token));
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<UnitThreadTracesResponse> responsePtr = std::make_unique<UnitThreadTracesResponse>();
    UnitThreadTracesResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    // query data from database, code in core package

    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
}
}