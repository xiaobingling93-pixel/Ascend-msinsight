/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "ServerLog.h"
#include "FileSelector.h"
#include "WsSessionManager.h"
#include "FilesGetHandler.h"

namespace Dic {
namespace Module {
namespace Global {
using namespace Dic::Server;
void FilesGetHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    FilesGetRequest &request = dynamic_cast<FilesGetRequest &>(*requestPtr.get());
    std::string sessionToken = request.token;
    ServerLog::Info("Files get start, token = ", StringUtil::AnonymousString(sessionToken));
    if (!WsSessionManager::Instance().CheckSession(sessionToken)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(sessionToken),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(sessionToken);
    std::unique_ptr<FilesGetResponse> responsePtr = std::make_unique<FilesGetResponse>();
    FilesGetResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    response.body.path = request.params.path;
    FileSelector::GetFoldersAndFiles(request.params.path, response.body.childrenFolders, response.body.childrenFiles);
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // Global
} // Module
} // Dic