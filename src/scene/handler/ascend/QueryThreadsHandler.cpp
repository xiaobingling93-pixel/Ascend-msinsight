//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#include "QueryThreadsHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"

namespace Dic {
namespace Scene {
using namespace Dic::Server;
void QueryThreadsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) {
    UnitThreadsRequest &request = dynamic_cast<UnitThreadsRequest &>(*requestPtr.get());
    std::string token = request.token;
    ServerLog::Info("Hdc list device start, token = ", StringUtil::AnonymousString(token));
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<UnitThreadsResponse> responsePtr = std::make_unique<UnitThreadsResponse>();
    UnitThreadsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    // query data from database, code in core package

    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
}
}