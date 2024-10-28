/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "WsSessionManager.h"
#include "ProjectExplorerManager.h"
#include "DeleteProjectExplorerInfoHandler.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Global;

bool DeleteProjectExplorerInfoHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    auto &request = dynamic_cast<ProjectExplorerInfoDeleteRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<ProjectExplorerInfoDeleteResponse> responsePtr =
            std::make_unique<ProjectExplorerInfoDeleteResponse>();
    ProjectExplorerInfoDeleteResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    bool res = ProjectExplorerManager::Instance()
            .DeleteProjectAndFilePath(request.params.projectName, request.params.dataPath);
    SetResponseResult(response, res);
    session.OnResponse(std::move(responsePtr));
    return res;
}
} // end of namespace Module
} // Dic
