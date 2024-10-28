/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "WsSessionManager.h"
#include "ProjectExplorerManager.h"
#include "BaselineManager.h"
#include "ParserFactory.h"
#include "ClearProjectExplorerHandler.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Global;

bool ClearProjectExplorerHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    auto &request = dynamic_cast<ProjectExplorerInfoClearRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<ProjectExplorerInfoClearResponse> responsePtr =
            std::make_unique<ProjectExplorerInfoClearResponse>();
    ProjectExplorerInfoClearResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    // 清空当前已加载数据
    ParserFactory::Reset();
    BaselineManager::Instance().Reset();
    // 清空db
    bool res = ProjectExplorerManager::Instance().ClearProjectExplorer();
    SetResponseResult(response, res);
    session.OnResponse(std::move(responsePtr));
    return res;
}
} // end of namespace Module
} // Dic