/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "FileUtil.h"
#include "WsSessionManager.h"
#include "SceneManager.h"
#include "ConfigSetHandler.h"

namespace Dic {
namespace Scene {
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
        SceneManager::Instance().SetGlobalConfig(request.params.globalConfig.value());
    }
    if (request.params.harmonyConfig.has_value()) {
        SceneManager::Instance().SetHarmonyConfig(request.params.harmonyConfig.value());
        response.body.isAlertMsg = !CheckDiskSize(request.params.harmonyConfig.value().dfx.dbDir);
    }
    response.body.configSetTime = TimeUtil::Instance().NowUTC();
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
}

bool ConfigSetHandler::CheckDiskSize(const std::string &path) const
{
    static const uint64_t ALERT_SIZE = 1024 * 1024 * 1024; // 1GB
    uint64_t size = Dic::FileUtil::GetDiskFreeSize(path);
    ServerLog::Info("Disk free size is ", size, " alertSize:", ALERT_SIZE);
    if (size > ALERT_SIZE) {
        return true;
    } else {
        return false;
    }
}
} // end of namespace Scene
} // Dic