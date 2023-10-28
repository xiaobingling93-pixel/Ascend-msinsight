/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "QueryMemoryOperatorHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "MemoryProtocolRequest.h"
#include "MemoryProtocolRespose.h"

namespace Dic {
namespace Module {
namespace Memory {
using namespace Dic::Server;
void QueryMemoryOperatorHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    MemoryOperatorRequest &request = dynamic_cast<MemoryOperatorRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Error("Failed to check session token , command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<MemoryOperatorResponse> responsePtr = std::make_unique<MemoryOperatorResponse>();
    MemoryOperatorResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = Timeline::DataBaseManager::Instance().GetMemoryDatabase(request.params.rankId);
    if (!database->QueryOperatorDetail(request.params, response.operatorDetails) or
        !database->QueryOperatorsTotalNum(request.params, response.totalNum)) {
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
