/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "DetailsService.h"
#include "SourceProtocolResponse.h"
#include "WsSessionManager.h"
#include "SourceProtocolRequest.h"
#include "QueryDetailsMemoryGraphHandler.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;

void QueryDetailsMemoryGraphHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<DetailsMemoryGraphRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<DetailsMemoryGraphResponse> responsePtr = std::make_unique<DetailsMemoryGraphResponse>();
    DetailsMemoryGraphResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    bool result = DetailsService::QueryMemoryGraph(request, response);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
}
}
}