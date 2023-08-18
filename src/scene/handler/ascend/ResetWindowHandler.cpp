//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#include "ResetWindowHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Scene {
using namespace Dic::Server;
using namespace Dic::Scene::Core;
void ResetWindowHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) {
    ResetWindowRequest &request = dynamic_cast<ResetWindowRequest &>(*requestPtr.get());
    std::string token = request.token;
    ServerLog::Info("Reset window, token = ", StringUtil::AnonymousString(token));
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<ResetWindowResponse> responsePtr = std::make_unique<ResetWindowResponse>();
    ResetWindowResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    // query data from database, code in core package

    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
}
}