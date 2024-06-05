/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "GetProjectExplorerInfoHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "ProjectExplorerManager.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Global;

void GetProjectExplorerInfoHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    auto &request = dynamic_cast<ProjectExplorerInfoGetRequest &>(*requestPtr.get());
    std::string sessionToken = request.token;
    if (!WsSessionManager::Instance().CheckSession(sessionToken)) {
        ServerLog::Error("Failed to check session, command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(sessionToken);
    std::unique_ptr<ProjectExplorerInfoGetResponse> responsePtr = std::make_unique<ProjectExplorerInfoGetResponse>();
    ProjectExplorerInfoGetResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    std::vector<ProjectExplorerInfo> infos = ProjectExplorerManager::Instance()
            .QueryProjectExplorer("", std::vector<std::string>());

    std::map<std::string, std::vector<std::string>> res;
    for (auto &item : infos) {
        res[item.projectName].push_back(item.fileName);
    }
    for (const auto &item : res) {
        Protocol::ProjectDirectoryInfo temp;
        temp.projectName = item.first;
        temp.fileName = item.second;
        response.body.projectDirectoryList.push_back(temp);
    }

    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // end of namespace Module
} // Dic
