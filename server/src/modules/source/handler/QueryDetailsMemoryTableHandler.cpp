/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "DetailsService.h"
#include "WsSessionManager.h"
#include "SourceProtocolRequest.h"
#include "QueryDetailsMemoryTableHandler.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;

bool QueryDetailsMemoryTableHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<DetailsMemoryTableRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<DetailsMemoryTableResponse> responsePtr =
            std::make_unique<DetailsMemoryTableResponse>();
    DetailsMemoryTableResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    if (auto [isVaild, errMsg] = request.params.Vaild(); isVaild == false) {
        ServerLog::Error("Parameter of command ", request.command, " is invaild, error:", errMsg);
        SetResponseResult(response, false, errMsg, ErrorCode::REQUEST_PARAMS_ERROR);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    bool result = DetailsService::QueryMemoryTable(request, response);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return true;
}
}
}
}