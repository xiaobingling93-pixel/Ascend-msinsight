/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "pch.h"
#include "FileSelector.h"
#include "WsSessionManager.h"
#include "FilesGetHandler.h"

namespace Dic {
namespace Module {
namespace Global {
using namespace Dic::Server;
bool FilesGetHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    FilesGetRequest &request = dynamic_cast<FilesGetRequest &>(*requestPtr.get());
    ServerLog::Info("Files get start");
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<FilesGetResponse> responsePtr = std::make_unique<FilesGetResponse>();
    FilesGetResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    response.body.path = request.params.path;
    FileSelector::GetFoldersAndFiles(request.params.path,
                                     response.body.childrenFolders,
                                     response.body.childrenFiles,
                                     response.body.exist);
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Global
} // Module
} // Dic