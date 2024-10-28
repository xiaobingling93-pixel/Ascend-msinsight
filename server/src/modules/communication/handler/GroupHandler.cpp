/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "pch.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "GroupHandler.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic;
using namespace Dic::Server;

bool GroupHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    MatrixGroupRequest &request = dynamic_cast<MatrixGroupRequest &>(*requestPtr.get());
    std::unique_ptr<Protocol::MatrixGroupResponse> responsePtr =
            std::make_unique<Protocol::MatrixGroupResponse>();
    MatrixGroupResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    auto database = Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    if (!database->GetGroups(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get group response data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Communication
} // Module
} // Dic