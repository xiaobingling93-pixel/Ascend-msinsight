/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "WsSessionManager.h"
#include "TraceTime.h"
#include "DataBaseManager.h"
#include "QueryUnitCounterHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void QueryUnitCounterHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    UnitCounterRequest &request = dynamic_cast<UnitCounterRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<UnitCounterResponse> responsePtr = std::make_unique<UnitCounterResponse>();
    UnitCounterResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", request.params.rankId);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    bool result = database->QueryUnitCounter(request.params, TraceTime::Instance().GetStartTime(), response.body.data);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic