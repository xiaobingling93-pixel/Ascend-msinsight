/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "QueryDetailsLoadInfoHandler.h"
#include "SourceFileParser.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "SourceProtocolRequest.h"
#include "SourceProtocolResponse.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;

void QueryDetailsLoadInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<SourceDetailsLoadInfoRequest &>(*requestPtr);
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Error("Failed to check session token , command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<DetailsLoadInfoResponse> responsePtr = std::make_unique<DetailsLoadInfoResponse>();
    DetailsLoadInfoResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    bool result = SourceFileParser::Instance().GetDetailsLoadInfo(response.body);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
}
}
}