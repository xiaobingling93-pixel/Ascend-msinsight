/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "WsSessionManager.h"
#include "ModuleManager.h"
#include "ConfigGetHandler.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
void ConfigGetHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    ConfigGetRequest &request = dynamic_cast<ConfigGetRequest &>(*requestPtr.get());
    std::string sessionToken = request.token;
    ServerLog::Info("Config get start, token = ", StringUtil::AnonymousString(sessionToken));
    if (!WsSessionManager::Instance().CheckSession(sessionToken)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(sessionToken),
            ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(sessionToken);
    std::unique_ptr<ConfigGetResponse> responsePtr = std::make_unique<ConfigGetResponse>();
    ConfigGetResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    int moduleMask = request.params.moduleMask;
    if ((moduleMask == -1) || (moduleMask & static_cast<int>(ModuleType::GLOBAL))) {
        response.body.globalConfig = ModuleManager::Instance().GetGlobalConfig();
    }
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
}
} // end of namespace Module
} // Dic