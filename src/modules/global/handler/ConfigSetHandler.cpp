/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "FileUtil.h"
#include "WsSessionManager.h"
#include "ModuleManager.h"
#include "ConfigSetHandler.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
void ConfigSetHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    ConfigSetRequest &request = dynamic_cast<ConfigSetRequest &>(*requestPtr.get());
    std::string sessionToken = request.token;
    ServerLog::Info("config set start, token = ", StringUtil::AnonymousString(sessionToken));
    if (!WsSessionManager::Instance().CheckSession(sessionToken)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(sessionToken),
            ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(sessionToken);
    std::unique_ptr<ConfigSetResponse> responsePtr = std::make_unique<ConfigSetResponse>();
    ConfigSetResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    if (request.params.globalConfig.has_value()) {
        ModuleManager::Instance().SetGlobalConfig(request.params.globalConfig.value());
    }
    response.body.configSetTime = TimeUtil::Instance().NowUTC();
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
}
} // end of namespace Module
} // Dic