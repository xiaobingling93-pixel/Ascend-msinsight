/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "pch.h"
#include "WsSessionManager.h"
#include "HeartCheckHandler.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;

bool HeartCheckHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    auto &request = dynamic_cast<HeartCheckRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<TokenHeartCheckResponse> responsePtr = std::make_unique<TokenHeartCheckResponse>();
    TokenHeartCheckResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // end of namespace Module
} // Dic