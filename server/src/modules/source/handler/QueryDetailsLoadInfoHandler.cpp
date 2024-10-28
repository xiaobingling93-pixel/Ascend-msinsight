/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "DetailsService.h"
#include "WsSessionManager.h"
#include "SourceProtocolRequest.h"
#include "SourceProtocolResponse.h"
#include "QueryDetailsLoadInfoHandler.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;

bool QueryDetailsLoadInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<SourceDetailsLoadInfoRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<DetailsLoadInfoResponse> responsePtr = std::make_unique<DetailsLoadInfoResponse>();
    DetailsLoadInfoResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    bool result = DetailsService::QueryDetailsLoadInfo(request, response);
    SetResponseResult(response, result);
    session.OnResponse(std::move(responsePtr));
    return result;
}
}
}
}