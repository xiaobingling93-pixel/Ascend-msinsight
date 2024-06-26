/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "QueryDetailsMemoryTableHandler.h"
#include "SourceFileParser.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "SourceProtocolRequest.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;

void QueryDetailsMemoryTableHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<DetailsMemoryTableRequest &>(*requestPtr);
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Error("Failed to check session token , command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<DetailsMemoryTableResponse> responsePtr =
            std::make_unique<DetailsMemoryTableResponse>();
    DetailsMemoryTableResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    bool result = SourceFileParser::Instance().GetDetailsMemoryTable(request.params.blockId,
                                                                     response.body);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
}
}
}