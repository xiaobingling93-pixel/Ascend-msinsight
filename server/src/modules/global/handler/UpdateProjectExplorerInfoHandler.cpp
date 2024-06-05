/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "UpdateProjectExplorerInfoHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "ProjectExplorerManager.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Global;

void UpdateProjectExplorerInfoHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    auto &request = dynamic_cast<ProjectExplorerInfoUpdateRequest &>(*requestPtr.get());
    std::string sessionToken = request.token;
    if (!WsSessionManager::Instance().CheckSession(sessionToken)) {
        ServerLog::Error("Failed to check session, command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(sessionToken);
    std::unique_ptr<ProjectExplorerInfoUpdateResponse> responsePtr =
            std::make_unique<ProjectExplorerInfoUpdateResponse>();
    ProjectExplorerInfoUpdateResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    bool res = ProjectExplorerManager::Instance().UpdateProjectName(request.params.oldProjectName,
                                                                    request.params.newProjectName);
    SetResponseResult(response, res);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // end of namespace Module
} // Dic