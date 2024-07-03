/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "CheckProjectConflictHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "ProjectExplorerManager.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Global;

void Dic::Module::CheckProjectConflictHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    auto &request = dynamic_cast<ProjectConflictCheckRequest &>(*requestPtr.get());
    std::string sessionToken = request.token;
    if (!WsSessionManager::Instance().CheckSession(sessionToken)) {
        ServerLog::Error("Failed to check session, command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(sessionToken);
    std::unique_ptr<ProjectConflictCheckResponse> responsePtr =
            std::make_unique<ProjectConflictCheckResponse>();
    ProjectConflictCheckResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.ConvertToRealPath(errorMsg)) {
        ServerLog::Error("[Operator]Failed to check request parameter. ", errorMsg);
        SetResponseResult(response, false, errorMsg);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    ProjectExplorerManager::Instance().CheckProjectConflict(request.params.projectName,
                                                            request.params.dataPath, response.body);
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
}
}